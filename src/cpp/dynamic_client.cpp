#include "dynamic_client.h"
#include "exceptions.h"

using namespace std;
using namespace ni;

DynamicClient::DynamicClient(const string& target)
{
	shared_ptr<grpc::ChannelCredentials> insecure_credentials = grpc::InsecureChannelCredentials();
	_channel = grpc::CreateChannel(target, insecure_credentials);
	
	_reflection_db = new grpc::ProtoReflectionDescriptorDatabase(_channel);
	_descriptor_pool = new google::protobuf::DescriptorPool(_reflection_db);
	_message_factory = new google::protobuf::DynamicMessageFactory(_descriptor_pool);
}

DynamicClient::~DynamicClient()
{
	delete _message_factory;
	delete _descriptor_pool;
	delete _reflection_db;
}

string DynamicClient::Query(const string& service, const string& method, const string& request)
{
	const google::protobuf::ServiceDescriptor* service_descriptor = _descriptor_pool->FindServiceByName(service);
	if (service_descriptor == nullptr)
	{
		throw ServiceNotFoundException(service);
	}
	const google::protobuf::MethodDescriptor* method_descriptor = service_descriptor->FindMethodByName(method);
	if (method_descriptor == nullptr)
	{
		throw MethodNotFoundException(method);
	}
	const google::protobuf::Descriptor* request_descriptor = method_descriptor->input_type();
	const google::protobuf::Descriptor* response_descriptor = method_descriptor->output_type();
	const google::protobuf::Message* request_prototype = _message_factory->GetPrototype(request_descriptor);
	const google::protobuf::Message* response_prototype = _message_factory->GetPrototype(response_descriptor);

	unique_ptr<google::protobuf::Message> request_message(request_prototype->New());
	unique_ptr<google::protobuf::Message> response_message(response_prototype->New());

	google::protobuf::util::JsonStringToMessage(request, request_message.get());

	string endpoint = string("/") + service + "/" + method;
	grpc::internal::RpcMethod rpc_method(endpoint.c_str(), grpc::internal::RpcMethod::NORMAL_RPC);
	grpc::internal::BlockingUnaryCall(_channel.get(), rpc_method, &grpc::ClientContext(), *request_message, response_message.get());

	string response;
	google::protobuf::util::JsonOptions json_options;
	json_options.always_print_primitive_fields = true;
	json_options.preserve_proto_field_names = true;
	google::protobuf::util::MessageToJsonString(*response_message, &response, json_options);

	return response;
}
