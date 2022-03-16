
#include "unary_unary_json_client.h"

#include <chrono>
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_set>

#include <google/protobuf/descriptor.h>
#include <grpcpp/grpcpp.h>
#include <grpcpp/generic/generic_stub.h>

#include "exceptions.h"
#include "json_client_base.h"
#include "json_serializer.h"

using grpc::ByteBuffer;
using grpc::CompletionQueue;
using grpc::ChannelCredentials;
using grpc::Status;
using std::runtime_error;
using std::shared_ptr;
using std::string;
using std::unique_ptr;
using std::chrono::system_clock;

namespace ni {
namespace grpc_json_client {

UnaryUnaryJsonClient::UnaryUnaryJsonClient(
    const std::string& target, const shared_ptr<ChannelCredentials>& credentials
) :
    JsonClientBase(target, credentials),
    _stub(channel)
{}

UnaryUnaryJsonClient::~UnaryUnaryJsonClient() {
    for (AsyncCallData* async_call : _tags) {
        delete async_call;
    }
}

void* UnaryUnaryJsonClient::StartAsyncCall(
    const string& service_name,
    const string& method_name,
    const string& request_json,
    const system_clock::time_point& deadline
) {
    unique_ptr<AsyncCallData> async_call = std::make_unique<AsyncCallData>();
    async_call->method_type = FindMethod(service_name, method_name, deadline);
    string endpoint = "/" + service_name + "/" + method_name;
    JsonSerializer serializer;
    ByteBuffer serialized_request = {
        serializer.SerializeMessage(async_call->method_type->input_type(), request_json)
    };
    async_call->context.set_deadline(deadline);
    async_call->response_reader = {
        _stub.PrepareUnaryCall(
            &async_call->context, endpoint, serialized_request, &async_call->completion_queue)
    };
    async_call->response_reader->StartCall();
    _tags.insert(async_call.get());
    return async_call.release();
}

string UnaryUnaryJsonClient::FinishAsyncCall(void* tag, const system_clock::time_point& deadline) {
    unique_ptr<AsyncCallData> async_call(static_cast<AsyncCallData*>(tag));
    if (!_tags.erase(async_call.get())) {
        async_call.release();  // don't delete whatever it is we're holding
        string message = {
            "An active remote procedure call was not found for the specified tag."
        };
        throw InvalidArgumentException(message);
    }
    ByteBuffer serialized_response;
    Status status;
    async_call->response_reader->Finish(&serialized_response, &status, async_call.get());
    void* next_tag = nullptr;
    bool ok = false;
    CompletionQueue::NextStatus next_status = {
        async_call->completion_queue.AsyncNext(&next_tag, &ok, deadline)
    };
    switch (next_status) {
    case CompletionQueue::NextStatus::SHUTDOWN:
        // we shouldn't reach this point since the completion queue
        // should only be drained in the destructor
        throw runtime_error("The completion queue shut down unexpectedly.");
    case CompletionQueue::NextStatus::GOT_EVENT:
        if (next_tag != async_call.get()) {
            // each call gets it's own completion queue so this shouldn't happen
            string message = {
                "The specified tag did not match the tag returned from the completion queue."
            };
            throw runtime_error(message);
        }
        if (ok) {
            if (!status.ok()) {
                string summary("An error occurred during the remote procedure call.");
                throw RemoteProcedureCallException(status, summary);
            }
            JsonSerializer serializer;
            return serializer.DeserializeMessage(
                async_call->method_type->output_type(), &serialized_response);
        }
        // if we reach this point then the completion queue is shutting down
        throw runtime_error("The completion queue is shutting down unexpectedly.");
    case CompletionQueue::NextStatus::TIMEOUT:
        throw TimeoutException(
            "Timed out while waiting for the remote procedure call to complete.");
    default:
        // throw exception for unhandled case
        throw runtime_error("An unknown status was returned from the completion queue.");
    }
}

UnaryUnaryJsonClient::AsyncCallData::AsyncCallData() : method_type(nullptr) {}

UnaryUnaryJsonClient::AsyncCallData::~AsyncCallData() {
    // shutdown and drain completion queue
    completion_queue.Shutdown();
    void* next_tag = nullptr;
    bool ok = false;
    while (completion_queue.Next(&next_tag, &ok)) {}
}

}  // namespace grpc_json_client
}  // namespace ni
