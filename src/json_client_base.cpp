
#include "json_client_base.h"

#include <chrono>
#include <memory>
#include <stdexcept>
#include <string>

#include "google/protobuf/descriptor.h"
#include "google/protobuf/descriptor.pb.h"
#include "google/protobuf/descriptor_database.h"
#include "google/protobuf/repeated_field.h"
#include "grpcpp/grpcpp.h"

#include "exceptions.h"
#include "json_serializer.h"
#include "reflection.grpc.pb.h"
#include "reflection.pb.h"

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
using grpc::StatusCode;
using grpc::reflection::v1alpha::FileDescriptorResponse;
using grpc::reflection::v1alpha::ServerReflection;
using grpc::reflection::v1alpha::ServerReflectionRequest;
using grpc::reflection::v1alpha::ServerReflectionResponse;
using grpc::reflection::v1alpha::ServiceResponse;
using std::chrono::system_clock;
using std::runtime_error;
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
    _pool(std::make_unique<DescriptorPool>(_database.get()))
{}

void JsonClientBase::ResetDescriptorDatabase() {
    _database = std::make_unique<SimpleDescriptorDatabase>();
    _pool = std::make_unique<DescriptorPool>(_database.get());
}

void JsonClientBase::FillDescriptorDatabase(const system_clock::time_point& deadline) {
    ServerReflectionRequest request;
    request.set_list_services("");  // content will not be checked
    ServerReflectionResponse response;
    QueryReflectionService(request, &response, deadline);
    RepeatedPtrField<ServiceResponse> services = response.list_services_response().service();
    for (ServiceResponse service : services) {
        FetchFileDescriptors(service.name(), deadline);
    }
}

const MethodDescriptor* JsonClientBase::FindMethod(
    const string& service_name,
    const string& method_name,
    const system_clock::time_point& deadline
) {
    const ServiceDescriptor* service_descriptor = _pool->FindServiceByName(service_name);
    if (service_descriptor == nullptr) {
        // try to fetch the file descriptors for the missing service
        try {
            FetchFileDescriptors(service_name, deadline);
        } catch (const ReflectionServiceException& ex) {
            // if the file descriptor is not found rethrow as ServiceNotFoundException
            if (ex.status().error_code() == StatusCode::NOT_FOUND) {
                std::throw_with_nested(ServiceNotFoundException(service_name));
            }
            throw;
        }
    }
    service_descriptor = _pool->FindServiceByName(service_name);
    if (service_descriptor == nullptr) {
        string message = {
            "The descriptor database failed to find a service descriptor for the requested "
            "service despite a successful query for the required file descriptors."
        };
        throw runtime_error(message);
    }
    const MethodDescriptor* method_descriptor = service_descriptor->FindMethodByName(method_name);
    if (method_descriptor == nullptr) {
        throw MethodNotFoundException(method_name);
    }
    return method_descriptor;
}

void JsonClientBase::QueryReflectionService(
    const ServerReflectionRequest& request,
    ServerReflectionResponse* response,
    const system_clock::time_point& deadline
) {
    // create stream
    ServerReflection::Stub stub(channel);
    ClientContext context;
    context.set_deadline(deadline);
    unique_ptr<ClientReflectionReaderWriter> stream = stub.ServerReflectionInfo(&context);

    // query
    bool sent_request = stream->Write(request);
    bool sent_writes_done = stream->WritesDone();
    bool got_response = stream->Read(response);
    Status status = stream->Finish();

    // check for errors
    if (!sent_request) {
        string summary("Failed to initiate communication with the host.");
        throw RemoteProcedureCallException(status, summary);
    }
    if (!sent_writes_done) {
        string summary("The connection with the host was interrupted.");
        throw RemoteProcedureCallException(status, summary);
    }
    if (status.error_code() == StatusCode::UNIMPLEMENTED) {
        string summary("The reflection service is not running on the host.");
        throw RemoteProcedureCallException(status, summary);
    }
    if (!status.ok()) {
        string summary("An error occurred while communicating with the host.");
        throw RemoteProcedureCallException(status, summary);
    }
    if (!got_response) {
        // failsafe to make sure we don't continue if we got StatusCode OK but somehow didn't get a
        // response
        string message("Failed to receive a response from the host despite gRPC StatusCode OK.");
        throw runtime_error(message);
    }
    if (response->has_error_response()) {
        grpc::Status status = {
            static_cast<grpc::StatusCode>(response->error_response().error_code()),
            response->error_response().error_message()
        };
        string details;
        JsonSerializer serializer;
        try {
            details = "Sent message \"" + serializer.MessageToJsonString(request) + '\"';
        } catch (SerializationException) {
            // shouldn't let this hide the original error if it happens
        }
        string summary("The reflection service reported an error.");
        throw ReflectionServiceException(status, summary, details);
    }
}

void JsonClientBase::FetchFileDescriptors(
    const string& symbol,
    const system_clock::time_point& deadline
) {
    ServerReflectionRequest request;
    request.set_file_containing_symbol(symbol);
    ServerReflectionResponse response;
    QueryReflectionService(request, &response, deadline);
    const FileDescriptorResponse& file_descriptor_response = response.file_descriptor_response();
    for (string serialized_file_descriptor : file_descriptor_response.file_descriptor_proto()) {
        FileDescriptorProto file_descriptor_proto;
        file_descriptor_proto.ParseFromString(serialized_file_descriptor);
        // the database will dump warning strings to stdout if a file descriptor already exists so
        // we avoid that by checking if we already have the file descriptor before adding it
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
