#pragma once

#include <google/protobuf/descriptor.h>
#include <google/protobuf/descriptor_database.h>
#include <google/protobuf/dynamic_message.h>
#include <google/protobuf/util/json_util.h>
#include <grpc/grpc.h>
#include <grpcpp/impl/client_unary_call.h>

#include "proto_reflection_descriptor_database.h"

namespace ni
{
	class DynamicClient
	{
	public:
		DynamicClient(const std::string& target);
		~DynamicClient();

		std::string Query(const std::string & service, const std::string & method, const std::string & request);

	private:
		std::shared_ptr<grpc::Channel> _channel;
		grpc::ProtoReflectionDescriptorDatabase* _reflection_db;
		google::protobuf::DescriptorPool* _descriptor_pool;
		google::protobuf::DynamicMessageFactory* _message_factory;
	};
}
