#pragma once

#include <memory>

#include <google/protobuf/descriptor.h>
#include <google/protobuf/descriptor_database.h>
#include <grpc/grpc.h>
#include "proto_reflection_descriptor_database.h"

namespace ni
{
	class DynamicClient
	{
	protected:
		std::shared_ptr<grpc::Channel> channel;

	private:
		std::shared_ptr<grpc::ProtoReflectionDescriptorDatabase> _reflection_db;
		std::shared_ptr<google::protobuf::DescriptorPool> _descriptor_pool;
		std::shared_ptr<google::protobuf::DynamicMessageFactory> _message_factory;

	public:
		DynamicClient(const std::string& target);
		~DynamicClient() = default;

		const google::protobuf::MethodDescriptor* FindMethod(const std::string& service_name, const std::string& method_name);
		grpc::ByteBuffer SerializeMessage(const google::protobuf::Descriptor* message_type, const std::string& message_json);
		std::string DeserializeMessage(const google::protobuf::Descriptor* message_type, grpc::ByteBuffer& serialized_message);

	private:
		std::unique_ptr<google::protobuf::Message> CreateMessage(const google::protobuf::Descriptor* message_type);
	};
}
