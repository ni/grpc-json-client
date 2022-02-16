#include "unary_unary_json_client.h"

#include "exceptions.h"
#include "json_serializer.h"

using grpc::ByteBuffer;
using grpc::ChannelCredentials;
using grpc::CompletionQueue;
using grpc::ClientContext;
using grpc::GenericStub;
using grpc::Status;
using std::shared_ptr;
using std::string;

namespace ni
{
	namespace json_client
	{
		UnaryUnaryJsonClient::UnaryUnaryJsonClient(const std::string& target, const shared_ptr<ChannelCredentials>& credentials) :
			JsonClientBase(target, credentials),
			_stub(channel)
		{}

		void UnaryUnaryJsonClient::Write(const string& service_name, const string& method_name, const string& request_json)
		{
			_method_type = FindMethod(service_name, method_name);
			ByteBuffer serialized_request = JsonSerializer::SerializeMessage(_method_type->input_type(), request_json);
			string endpoint = string("/") + service_name + "/" + method_name;
			_context = std::make_unique<ClientContext>();
			_response_reader = _stub.PrepareUnaryCall(_context.get(), endpoint, serialized_request, &_completion_queue);
			_response_reader->StartCall();
			_response_reader->Finish(&_serialized_response, &_status, (void*)1);
		}

		string UnaryUnaryJsonClient::Read()
		{
			if (_response_reader)
			{
				_response_reader.reset();

				void* tag;
				bool ok = false;
				if (!_completion_queue.Next(&tag, &ok))
				{
					// todo
				}
				if (!ok)
				{
					// todo
				}

				_response = JsonSerializer::DeserializeMessage(_method_type->output_type(), _serialized_response);
			}
			return _response;
		}
	}
}
