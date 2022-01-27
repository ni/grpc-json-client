// grpc_json_client.cpp : Defines the entry point for the application.
//

#include "dynamic_client.h"

ni::DynamicClient::DynamicClient(const string& target)
{
	shared_ptr<grpc::ChannelCredentials> insecure_credentials = grpc::InsecureChannelCredentials();
	_channel = grpc::CreateChannel(target, insecure_credentials);
	
	_reflection_db = new grpc::ProtoReflectionDescriptorDatabase(_channel);
	_descriptor_pool = new google::protobuf::DescriptorPool(_reflection_db);
	_message_factory = new google::protobuf::DynamicMessageFactory(_descriptor_pool);
}

ni::DynamicClient::~DynamicClient()
{
	delete _message_factory;
	delete _descriptor_pool;
	delete _reflection_db;
}

string ni::DynamicClient::Query(const string& service, const string& method, const string& request)
{
	const google::protobuf::MethodDescriptor* method_descriptor = _descriptor_pool->FindMethodByName(service + "." + method);
	const google::protobuf::Descriptor* request_descriptor = method_descriptor->input_type();
	const google::protobuf::Descriptor* response_descriptor = method_descriptor->output_type();
	const google::protobuf::Message* request_prototype = _message_factory->GetPrototype(request_descriptor);
	const google::protobuf::Message* response_prototype = _message_factory->GetPrototype(response_descriptor);

	google::protobuf::Message* request_message = request_prototype->New();
	google::protobuf::Message* response_message = response_prototype->New();

	google::protobuf::util::JsonStringToMessage(request, request_message);

	string endpoint = string("/") + service + "/" + method;
	grpc::internal::RpcMethod rpc_method(endpoint.c_str(), grpc::internal::RpcMethod::NORMAL_RPC);
	grpc::internal::BlockingUnaryCall(_channel.get(), rpc_method, &grpc::ClientContext(), *request_message, response_message);

	string response;
	google::protobuf::util::JsonOptions json_options;
	json_options.always_print_primitive_fields = true;
	json_options.preserve_proto_field_names = true;
	google::protobuf::util::MessageToJsonString(*response_message, &response, json_options);

	delete request_message;
	delete response_message;

	return response;
}
