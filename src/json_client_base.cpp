#include "json_client_base.h"

#include "exceptions.h"
#include "reflection.pb.h"
#include "reflection.grpc.pb.h"

using google::protobuf::FileDescriptorProto;
using google::protobuf::MethodDescriptor;
using google::protobuf::RepeatedPtrField;
using google::protobuf::ServiceDescriptor;
using grpc::ChannelCredentials;
using grpc::ClientContext;
using grpc::ClientReaderWriter;
using grpc::reflection::v1alpha::FileDescriptorResponse;
using grpc::reflection::v1alpha::ServerReflection;
using grpc::reflection::v1alpha::ServerReflectionRequest;
using grpc::reflection::v1alpha::ServerReflectionResponse;
using grpc::reflection::v1alpha::ServiceResponse;
using std::shared_ptr;
using std::string;
using std::unique_ptr;

namespace ni
{
    namespace json_client
    {
        JsonClientBase::JsonClientBase(const string& target, const shared_ptr<ChannelCredentials>& credentials) :
            _descriptor_pool(&_reflection_db)
        {
            channel = CreateChannel(target, credentials);
        }

        void JsonClientBase::QueryReflectionService()
        {
            // request server to list services
            ServerReflection::Stub reflection_stub(channel);
            ClientContext context;
            unique_ptr<ClientReaderWriter<ServerReflectionRequest, ServerReflectionResponse>> stream = reflection_stub.ServerReflectionInfo(&context);
            ServerReflectionRequest request;
            request.set_list_services("");  // content will not be checked
            stream->Write(request);

            // read services
            ServerReflectionResponse response;
            stream->Read(&response);
            RepeatedPtrField<ServiceResponse> services = response.list_services_response().service();

            // request file descriptors for services
            for (ServiceResponse service : services)
            {
                request.set_file_containing_symbol(service.name());
                stream->Write(request);
                stream->Read(&response);
                const FileDescriptorResponse& file_descriptor_response = response.file_descriptor_response(); 
                for (string serialized_file_descriptor : file_descriptor_response.file_descriptor_proto())
                {
                    FileDescriptorProto file_descriptor_proto;
                    file_descriptor_proto.ParseFromString(serialized_file_descriptor);
                    if (!_reflection_db.FindFileByName(file_descriptor_proto.name(), &file_descriptor_proto))
                    {
                        _reflection_db.Add(file_descriptor_proto);
                    }
                }
            }

            // close stream
            stream->WritesDone();
            grpc::Status status = stream->Finish();
            if (!status.ok())
            {
                string summary("Failed to retreive file descriptors from the host. Ensure the reflection service is running and reachable.\n\n");
                throw RpcException(summary + status.error_message());
            }
        }

        const MethodDescriptor* JsonClientBase::FindMethod(const string& service_name, const string& method_name) const
        {
            const ServiceDescriptor* service_descriptor = _descriptor_pool.FindServiceByName(service_name);
            if (service_descriptor == nullptr)
            {
                throw ServiceDescriptorNotFoundException(service_name);
            }
            const MethodDescriptor* method_descriptor = service_descriptor->FindMethodByName(method_name);
            if (method_descriptor == nullptr)
            {
                throw MethodDescriptorNotFoundException(method_name);
            }
            return method_descriptor;
        }
    }
}
