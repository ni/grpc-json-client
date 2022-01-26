// grpc_json_client.cpp : Defines the entry point for the application.
//

#include <google/protobuf/descriptor.h>
#include <google/protobuf/descriptor_database.h>
#include <google/protobuf/dynamic_message.h>
#include <google/protobuf/util/json_util.h>
#include <grpc/grpc.h>
#include <grpcpp/impl/client_unary_call.h>

#include "grpc_dynamic_client.h"
#include "proto_reflection_descriptor_database.h"

using namespace std;

int main()
{
	shared_ptr<grpc::ChannelCredentials> insecure_credentials = grpc::InsecureChannelCredentials();
	shared_ptr<grpc::Channel> channel = grpc::CreateChannel("localhost:31763", insecure_credentials);

	grpc::ProtoReflectionDescriptorDatabase reflection_db(channel);
	google::protobuf::DescriptorPool descriptor_pool(&reflection_db);
	google::protobuf::DynamicMessageFactory message_factory(&descriptor_pool);

	const google::protobuf::MethodDescriptor* method_descriptor = descriptor_pool.FindMethodByName("nirfsa_grpc.NiRFSA.Init");
	const google::protobuf::Descriptor* request_descriptor = method_descriptor->input_type();
	const google::protobuf::Descriptor* response_descriptor = method_descriptor->output_type();
	const google::protobuf::Message* request_prototype = message_factory.GetPrototype(request_descriptor);
	const google::protobuf::Message* response_prototype = message_factory.GetPrototype(response_descriptor);
	google::protobuf::Message* request = request_prototype->New();
	google::protobuf::Message* response = response_prototype->New();
	google::protobuf::util::JsonStringToMessage("{\"resource_name\": \"VST2_01\"}", request);

	grpc::internal::RpcMethod method("/nirfsa_grpc.NiRFSA/Init", grpc::internal::RpcMethod::NORMAL_RPC);
	grpc::internal::BlockingUnaryCall(channel.get(), method, &grpc::ClientContext(), *request, response);

	string response_json;
	google::protobuf::util::JsonOptions json_options;
	json_options.always_print_primitive_fields = true;
	json_options.preserve_proto_field_names = true;
	google::protobuf::util::MessageToJsonString(*response, &response_json, json_options);

	cout << response_json << endl;

	return 0;
}
