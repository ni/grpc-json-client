#include "unary_unary_dynamic_client.h"

#include "exceptions.h"

using grpc::ByteBuffer;
using grpc::CompletionQueue;
using grpc::ClientContext;
using grpc::GenericStub;
using grpc::Status;
using std::string;

namespace ni
{
	UnaryUnaryDynamicClient::UnaryUnaryDynamicClient(const std::string& target) : DynamicClient(target)
	{
		_stub = std::make_unique<GenericStub>(channel);
		_completion_queue = std::make_unique<CompletionQueue>();
	}

	void UnaryUnaryDynamicClient::Write(const string& service_name, const string& method_name, const string& request_json)
	{
		_method_type = FindMethod(service_name, method_name);
		ByteBuffer serialized_request = SerializeMessage(_method_type->input_type(), request_json);
		string endpoint = string("/") + service_name + "/" + method_name;
		_context = std::make_unique<ClientContext>();
		_response_reader = _stub->PrepareUnaryCall(_context.get(), endpoint, serialized_request, _completion_queue.get());
		_response_reader->StartCall();
	}

	const string* UnaryUnaryDynamicClient::Read()
	{
		if (_response_reader)
		{
			ByteBuffer serialized_response;
			Status status;
			_response_reader->Finish(&serialized_response, &status, (void*)1);
			_response_reader.reset();

			void* got_tag;
			bool ok = false;
			if (!_completion_queue->Next(&got_tag, &ok))
			{
				// todo
			}
			if (!ok)
			{
				// todo
			}

			_response = DeserializeMessage(_method_type->output_type(), serialized_response);
		}
		return &_response;
	}
}

int main()
{
	ni::UnaryUnaryDynamicClient client("localhost:31763");
	//client.Debug("nirfsa_grpc.NiRFSA", "Init", "{\"resource_name\":\"VST2_01\"}");
	client.Write("nirfsa_grpc.NiRFSA", "Init", "{\"session_name\": \"VST\", \"resource_name\":\"VST2_01\"}");
	std::cout << *client.Read() << std::endl;
	client.Write("nirfsa_grpc.NiRFSA", "Close", "{\"vi\":{\"name\":\"VST\"}}");
	std::cout << *client.Read() << std::endl;
}