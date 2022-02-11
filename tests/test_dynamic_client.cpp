#include <gtest/gtest.h>
#include <google/protobuf/descriptor.h>

#include "dynamic_client.h"
#include "exceptions.h"

using google::protobuf::MethodDescriptor;
using std::string;

namespace ni
{
	const std::string host = "localhost:31763";

	class DynamicClientTest : public testing::Test
	{
	protected:
		DynamicClient client;

		DynamicClientTest() :
			client(host)
		{
		}

		void SetUp() override
		{
			client.QueryReflectionService();
		}
	};

	TEST_F(DynamicClientTest, FindMethodFindsValidMethod)
	{
		const MethodDescriptor* method;
		ASSERT_NO_THROW(
			method = client.FindMethod("nirfsa_grpc.NiRFSA", "Init");
		);
		EXPECT_EQ(method->full_name(), "nirfsa_grpc.NiRFSA.Init");
	}

	TEST_F(DynamicClientTest, FindMethodFailsOnInvalidServiceName)
	{
		ASSERT_THROW(
			client.FindMethod("invalid.service.name", "Init"),
			ServiceDescriptorNotFoundException
		);
	}

	TEST_F(DynamicClientTest, FindMethodFailsOnInvalidMethodName)
	{
		ASSERT_THROW(
			client.FindMethod("nirfsa_grpc.NiRFSA", "InvalidMethodName"),
			MethodDescriptorNotFoundException
		);
	}

	TEST_F(DynamicClientTest, SerializeMessageSerializesValidString)
	{
		const MethodDescriptor* method = client.FindMethod("nirfsa_grpc.NiRFSA", "Init");
	}
}