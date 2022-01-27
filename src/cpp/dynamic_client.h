#pragma once

#include <google/protobuf/descriptor.h>
#include <google/protobuf/descriptor_database.h>
#include <google/protobuf/dynamic_message.h>
#include <google/protobuf/util/json_util.h>
#include <grpc/grpc.h>
#include <grpcpp/impl/client_unary_call.h>

#include "dynamic_client.h"
#include "proto_reflection_descriptor_database.h"

using namespace std;

namespace ni
{
	class DynamicClient
	{
	public:
		DynamicClient(const string& target);
		~DynamicClient();

		string Query(const string& service, const string& method, const string& request);

	private:
		shared_ptr<grpc::Channel> _channel;
		grpc::ProtoReflectionDescriptorDatabase* _reflection_db;
		google::protobuf::DescriptorPool* _descriptor_pool;
		google::protobuf::DynamicMessageFactory* _message_factory;
	};
}
