#include "dynamic_client.h"
#include "exceptions.h"
#include <google/protobuf/dynamic_message.h>
#include <google/protobuf/util/json_util.h>

using namespace std;
using namespace ni;

DynamicClient::DynamicClient(const string& target)
{
	shared_ptr<grpc::ChannelCredentials> insecure_credentials = grpc::InsecureChannelCredentials();
	channel = grpc::CreateChannel(target, insecure_credentials);

	_reflection_db = make_shared<grpc::ProtoReflectionDescriptorDatabase>(channel);
	_descriptor_pool = make_shared<google::protobuf::DescriptorPool>(_reflection_db.get());
	_message_factory = make_shared<google::protobuf::DynamicMessageFactory>(_descriptor_pool.get());
}

const google::protobuf::MethodDescriptor* DynamicClient::FindMethod(const string& service_name, const string& method_name)
{
	const google::protobuf::ServiceDescriptor* service_descriptor = _descriptor_pool->FindServiceByName(service_name);
	if (service_descriptor == nullptr)
	{
		throw ServiceNotFoundException(service_name);
	}
	const google::protobuf::MethodDescriptor* method_descriptor = service_descriptor->FindMethodByName(method_name);
	if (method_descriptor == nullptr)
	{
		throw MethodNotFoundException(method_name);
	}
	return method_descriptor;
}

grpc::ByteBuffer DynamicClient::SerializeMessage(const google::protobuf::Descriptor* message_type, const string& message_json)
{
	unique_ptr<google::protobuf::Message> message = CreateMessage(message_type);
	google::protobuf::util::Status json_status = google::protobuf::util::JsonStringToMessage(message_json, message.get());
	if (!json_status.ok())
	{
		// todo
	}
	grpc::ByteBuffer serialized_message;
	bool own_buffer = false;
	grpc::Status serialize_status = grpc::GenericSerialize<grpc::ProtoBufferWriter, void>(*message, &serialized_message, &own_buffer);
	if (!serialize_status.ok())
	{
		// todo
	}
	return serialized_message;
}

string DynamicClient::DeserializeMessage(const google::protobuf::Descriptor* message_type, grpc::ByteBuffer& serialized_message)
{
	unique_ptr<google::protobuf::Message> message = CreateMessage(message_type);
	grpc::Status deserialize_status = grpc::GenericDeserialize<grpc::ProtoBufferReader, void>(&serialized_message, message.get());
	if (!deserialize_status.ok())
	{
		// todo
	}
	string response;
	google::protobuf::util::JsonOptions json_options;
	json_options.always_print_primitive_fields = true;
	json_options.preserve_proto_field_names = true;
	google::protobuf::util::Status json_status = google::protobuf::util::MessageToJsonString(*message, &response, json_options);
	if (!json_status.ok())
	{
		// todo
	}
	return response;
}

unique_ptr<google::protobuf::Message> DynamicClient::CreateMessage(const google::protobuf::Descriptor* message_type)
{
	const google::protobuf::Message* message_prototype = _message_factory->GetPrototype(message_type);
	return unique_ptr<google::protobuf::Message>(message_prototype->New());
}
