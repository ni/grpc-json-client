#include "unary_unary_json_client.h"

#include "exceptions.h"
#include "json_serializer.h"

using google::protobuf::MethodDescriptor;
using grpc::ByteBuffer;
using grpc::CompletionQueue;
using grpc::ChannelCredentials;
using grpc::GenericClientAsyncResponseReader;
using std::logic_error;
using std::shared_ptr;
using std::string;
using std::unique_ptr;
using std::chrono::milliseconds;
using std::chrono::system_clock;
using std::chrono::time_point;

namespace ni
{
	namespace json_client
	{
		UnaryUnaryJsonClient::UnaryUnaryJsonClient(const std::string& target, const shared_ptr<ChannelCredentials>& credentials) :
			JsonClientBase(target, credentials),
			_stub(channel)
		{}

		UnaryUnaryJsonClient::~UnaryUnaryJsonClient()
		{
			// shutdown and drain completion queue
			_completion_queue.Shutdown();
			void* tag = nullptr;
			bool ok = false;
			while (_completion_queue.Next(&tag, &ok));
		}

		void UnaryUnaryJsonClient::Write(const string& service_name, const string& method_name, const string& request_json)
		{
			const MethodDescriptor* method_type = FindMethod(service_name, method_name);
			string endpoint = string("/") + service_name + "/" + method_name;
			ByteBuffer serialized_request = JsonSerializer::SerializeMessage(method_type->input_type(), request_json);
			AsyncCallInfo* async_call = new AsyncCallInfo();
			async_call->method_type = method_type;
			unique_ptr<GenericClientAsyncResponseReader> response_reader = _stub.PrepareUnaryCall(&async_call->context, endpoint, serialized_request, &_completion_queue);
			response_reader->StartCall();
			response_reader->Finish(&async_call->serialized_response, &async_call->status, async_call);
		}
		
		string UnaryUnaryJsonClient::Read(int timeout)
		{
			void* tag = nullptr;
			bool ok = false;
			CompletionQueue::NextStatus next_status;
			if (timeout < 0)
			{
				next_status = static_cast<CompletionQueue::NextStatus>(_completion_queue.Next(&tag, &ok));
			}
			else
			{
				system_clock::time_point deadline = system_clock::now() + milliseconds(timeout);
				next_status = _completion_queue.AsyncNext(&tag, &ok, deadline);
			}
			switch (next_status)
			{
			case CompletionQueue::NextStatus::SHUTDOWN:
				// we shouldn't reach this point since the completion queue should only be drained in the destructor
				throw logic_error("A critical error occurred. The completion queue shut down unexpectedly.");
			case CompletionQueue::NextStatus::GOT_EVENT:
				if (ok)
				{
					unique_ptr<AsyncCallInfo> async_call(static_cast<AsyncCallInfo*>(tag));
					if (!async_call->status.ok())
					{
						string summary("An error occurred during the remote procedure call.\n\n");
						throw RpcException(summary + async_call->status.error_message());
					}
					const MethodDescriptor* method_type = async_call->method_type;
					return JsonSerializer::DeserializeMessage(method_type->output_type(), async_call->serialized_response);
				}
				// if we reach this point then the completion queue is shutting down
				throw logic_error("A critical error occurred. The completion queue is shutting down unexpectedly.");
			case CompletionQueue::NextStatus::TIMEOUT:
				throw TimeoutException("Remote procedure call timed out while waiting for a response from the host.");
			default:
				// throw exception for unhandled case
				throw logic_error("A critical error occurred. An unknown status was returned from the completion queue.");
			}
		}
	}
}
