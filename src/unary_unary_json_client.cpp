#include "unary_unary_json_client.h"

#include "exceptions.h"
#include "json_serializer.h"

using grpc::ByteBuffer;
using grpc::CompletionQueue;
using grpc::ChannelCredentials;
using grpc::Status;
using std::logic_error;
using std::shared_ptr;
using std::string;
using std::unique_ptr;
using std::chrono::milliseconds;
using std::chrono::system_clock;
using std::chrono::time_point;

namespace ni
{
    namespace grpc_json_client
    {
        UnaryUnaryJsonClient::UnaryUnaryJsonClient(const std::string& target, const shared_ptr<ChannelCredentials>& credentials) :
            JsonClientBase(target, credentials),
            _stub(channel)
        {}

        UnaryUnaryJsonClient::~UnaryUnaryJsonClient()
        {
            for (AsyncCallData* async_call : _tags)
            {
                delete async_call;
            }
        }

        void* UnaryUnaryJsonClient::StartAsyncCall(const string& service_name, const string& method_name, const string& request_json)
        {
            unique_ptr<AsyncCallData> async_call = std::make_unique<AsyncCallData>();
            async_call->method_type = FindMethod(service_name, method_name);
            string endpoint = string("/") + service_name + "/" + method_name;
            ByteBuffer serialized_request = JsonSerializer::SerializeMessage(async_call->method_type->input_type(), request_json);
            async_call->response_reader = (
                _stub.PrepareUnaryCall(&async_call->context, endpoint, serialized_request, &async_call->completion_queue)
            );
            async_call->response_reader->StartCall();
            _tags.insert(async_call.get());
            return async_call.release();
        }
        
        string UnaryUnaryJsonClient::FinishAsyncCall(void* tag, int timeout)
        {
            if (!_tags.erase(static_cast<AsyncCallData*>(tag)))
            {
                throw InvalidTagException("An active remote procedure call was not found for the specified tag.");
            }
            unique_ptr<AsyncCallData> async_call(static_cast<AsyncCallData*>(tag));
            ByteBuffer serialized_response;
            Status status;
            async_call->response_reader->Finish(&serialized_response, &status, async_call.get());
            void* next_tag = nullptr;
            bool ok = false;
            CompletionQueue::NextStatus next_status;
            if (timeout < 0)
            {
                next_status = static_cast<CompletionQueue::NextStatus>(async_call->completion_queue.Next(&next_tag, &ok));
            }
            else
            {
                system_clock::time_point deadline = system_clock::now() + milliseconds(timeout);
                next_status = async_call->completion_queue.AsyncNext(&next_tag, &ok, deadline);
            }
            switch (next_status)
            {
            case CompletionQueue::NextStatus::SHUTDOWN:
                // we shouldn't reach this point since the completion queue should only be drained in the destructor
                throw logic_error("The completion queue shut down unexpectedly.");
            case CompletionQueue::NextStatus::GOT_EVENT:
                if (next_tag != async_call.get())
                {
                    // each call gets it's own completion queue so this shouldn't happen
                    throw logic_error("The specified tag did not match the tag returned from the completion queue.");
                }
                if (ok)
                {
                    if (!status.ok())
                    {
                        string summary("An error occurred during the remote procedure call.\n\n");
                        throw RemoteProcedureCallException(summary + status.error_message());
                    }
                    return JsonSerializer::DeserializeMessage(async_call->method_type->output_type(), serialized_response);
                }
                // if we reach this point then the completion queue is shutting down
                throw logic_error("The completion queue is shutting down unexpectedly.");
            case CompletionQueue::NextStatus::TIMEOUT:
                throw TimeoutException("Remote procedure call timed out while waiting for a response from the host.");
            default:
                // throw exception for unhandled case
                throw logic_error("An unknown status was returned from the completion queue.");
            }
        }

        UnaryUnaryJsonClient::AsyncCallData::AsyncCallData() :
            method_type(nullptr)
        {}

        UnaryUnaryJsonClient::AsyncCallData::~AsyncCallData()
        {
            // shutdown and drain completion queue
            completion_queue.Shutdown();
            void* next_tag = nullptr;
            bool ok = false;
            while (completion_queue.Next(&next_tag, &ok));
        }
    }
}
