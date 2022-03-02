
#include "json_client_base.h"

#include <memory>
#include <string>

#include "exceptions.h"
#include "reflection.grpc.pb.h"

using google::protobuf::DescriptorPool;
using google::protobuf::FileDescriptorProto;
using google::protobuf::MethodDescriptor;
using google::protobuf::RepeatedPtrField;
using google::protobuf::ServiceDescriptor;
using google::protobuf::SimpleDescriptorDatabase;
using grpc::ChannelCredentials;
using grpc::ClientContext;
using grpc::ClientReaderWriter;
using grpc::Status;
using grpc::reflection::v1alpha::FileDescriptorResponse;
using grpc::reflection::v1alpha::ServerReflection;
using grpc::reflection::v1alpha::ServerReflectionRequest;
using grpc::reflection::v1alpha::ServerReflectionResponse;
using grpc::reflection::v1alpha::ServiceResponse;
using std::shared_ptr;
using std::string;
using std::unique_ptr;

using ClientReflectionReaderWriter =
    ClientReaderWriter<ServerReflectionRequest, ServerReflectionResponse>;

namespace ni {
namespace grpc_json_client {

JsonClientBase::JsonClientBase(
    const string& target, const shared_ptr<ChannelCredentials>& credentials
) : 
    channel(CreateChannel(target, credentials)),
    _database(std::make_unique<SimpleDescriptorDatabase>()),
    _pool(std::make_unique<DescriptorPool>(_database.get())) {
}

void JsonClientBase::ResetDescriptorDatabase() {
    _database = std::make_unique<SimpleDescriptorDatabase>();
    _pool = std::make_unique<DescriptorPool>(_database.get());
}

void JsonClientBase::FillDescriptorDatabase() {
    ServerReflectionRequest request;
    request.set_list_services("");  // content will not be checked
    ServerReflectionResponse response;
    QueryReflectionService(request, &response);
    RepeatedPtrField<ServiceResponse> services = response.list_services_response().service();
    for (ServiceResponse service : services) {
        FetchFileDescriptors(service.name());
    }
}

const MethodDescriptor* JsonClientBase::FindMethod(
    const string& service_name, const string& method_name
) {
    const ServiceDescriptor* service_descriptor = _pool->FindServiceByName(service_name);
    if (service_descriptor == nullptr) {
        try {
            FetchFileDescriptors(service_name);
        } catch (const ReflectionServiceException& ex) {
            if (ex.status().error_code() == grpc::StatusCode::NOT_FOUND) {
                throw ServiceNotFoundException(service_name);
            }
            throw;
        }
    }
    service_descriptor = _pool->FindServiceByName(service_name);
    if (service_descriptor == nullptr) {
        throw ServiceNotFoundException(service_name);
    }
    const MethodDescriptor* method_descriptor = service_descriptor->FindMethodByName(method_name);
    if (method_descriptor == nullptr) {
        throw MethodNotFoundException(method_name);
    }
    return method_descriptor;
}

void JsonClientBase::QueryReflectionService(
    const ServerReflectionRequest& request, ServerReflectionResponse* response
) {
    // create stream
    ServerReflection::Stub stub(channel);
    ClientContext context;
    // todo: what happens to this var if the service isn't running?
    unique_ptr<ClientReflectionReaderWriter> stream = stub.ServerReflectionInfo(&context);

    // send request
    if (!stream->Write(request)) {
        Status status = stream->Finish();
        string summary = {
            "The stream failed to initiate communication with the host. "
            "Ensure the reflection service is running and reachable."
        };
        throw RemoteProcedureCallException(status, summary);
    }
    stream->WritesDone();

    // read response
    if (!stream->Read(response)) {
        Status status = stream->Finish();
        string summary("The stream with the reflection service was interrupted.");
        throw RemoteProcedureCallException(status, summary);
    } else if (response->has_error_response()) {
        stream->Finish();
        grpc::StatusCode status_code = {
            static_cast<grpc::StatusCode>(response->error_response().error_code())
        };
        const string& error_message = response->error_response().error_message();
        grpc::Status status(status_code, error_message);
        throw ReflectionServiceException(status, "The reflection service reported an error.");
    }

    // close stream
    Status status = stream->Finish();
    if (!status.ok()) {
        string summary("An error occurred while communicating with the reflection service.");
        throw RemoteProcedureCallException(status, summary);
    }
}

void JsonClientBase::FetchFileDescriptors(const string& symbol) {
    ServerReflectionRequest request;
    request.set_file_containing_symbol(symbol);
    ServerReflectionResponse response;
    QueryReflectionService(request, &response);
    const FileDescriptorResponse& file_descriptor_response = response.file_descriptor_response();
    for (string serialized_file_descriptor : file_descriptor_response.file_descriptor_proto()) {
        FileDescriptorProto file_descriptor_proto;
        file_descriptor_proto.ParseFromString(serialized_file_descriptor);
        bool file_descriptor_in_database = {
            _database->FindFileByName(file_descriptor_proto.name(), &file_descriptor_proto)
        };
        if (!file_descriptor_in_database) {
            _database->Add(file_descriptor_proto);
        }
    }
}

}  // namespace grpc_json_client
}  // namespace ni
