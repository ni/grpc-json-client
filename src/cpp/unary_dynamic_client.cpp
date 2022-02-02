#include "unary_dynamic_client.h"
#include "exceptions.h"

using namespace std;
using namespace ni;

ni::UnaryDynamicClient::UnaryDynamicClient(const std::string& target) : DynamicClient(target)
{
	_stub = make_shared<grpc::GenericStub>(channel);
	_completion_queue = make_shared<grpc::CompletionQueue>();
}

void UnaryDynamicClient::Write(const string& service_name, const string& method_name, const string& request_json)
{
	_method_type = FindMethod(service_name, method_name);
	grpc::ByteBuffer serialized_request = SerializeMessage(_method_type->input_type(), request_json);
	string endpoint = string("/") + service_name + "/" + method_name;
	_context = make_shared<grpc::ClientContext>();
	_response_reader = _stub->PrepareUnaryCall(_context.get(), endpoint, serialized_request, _completion_queue.get());
	_response_reader->StartCall();
}

const string* UnaryDynamicClient::Read()
{
	if (_response_reader)
	{
		grpc::ByteBuffer serialized_response;
		grpc::Status status;
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

int main()
{
	cout << "hello" << endl;
	auto client = UnaryDynamicClient("localhost:31763");
	//client.Debug("nirfsa_grpc.NiRFSA", "Init", "{\"resource_name\":\"VST2_01\"}");
	client.Write("nirfsa_grpc.NiRFSA", "Init", "{\"resource_name\":\"VST2_01\"}");
	cout << *client.Read() << endl;
	client.Write("nirfsa_grpc.NiRFSA", "Close", "{\"vi\":{\"id\":1}}");
	cout << *client.Read() << endl;
}