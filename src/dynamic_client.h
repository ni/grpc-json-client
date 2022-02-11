#pragma once

#include <google/protobuf/descriptor.h>
#include <google/protobuf/descriptor_database.h>
#include <grpcpp/channel.h>

namespace ni
{
	class DynamicClient
	{
	protected:
		std::shared_ptr<grpc::Channel> channel;

	private:
		google::protobuf::SimpleDescriptorDatabase _reflection_db;
		google::protobuf::DescriptorPool _descriptor_pool;

	public:
		DynamicClient(const std::string& target);
		~DynamicClient() = default;

		// Populate descriptor pool with file descriptors for all services exposed by the reflection service on the host.
		void QueryReflectionService();

		// Search for a method in the descriptor database.
		const google::protobuf::MethodDescriptor* FindMethod(const std::string& service_name, const std::string& method_name);
	};
}
