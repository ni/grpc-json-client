#include <gtest/gtest.h>
#include <google/protobuf/descriptor.h>

#include "exceptions.h"
#include "json_client_base.h"

using google::protobuf::MethodDescriptor;
using std::string;

namespace ni
{
    namespace json_client
    {
        const std::string host = "localhost:31763";

        class JsonClientTest : public testing::Test
        {
        protected:
            JsonClientBase client;

            JsonClientTest() :
                client(host, grpc::InsecureChannelCredentials())
            {
            }

            void SetUp() override
            {
                client.QueryReflectionService();
            }
        };

        TEST_F(JsonClientTest, FindMethodFindsValidMethod)
        {
            const MethodDescriptor* method;
            ASSERT_NO_THROW(
                method = client.FindMethod("nirfsa_grpc.NiRFSA", "Init");
            );
            EXPECT_EQ(method->full_name(), "nirfsa_grpc.NiRFSA.Init");
        }

        TEST_F(JsonClientTest, FindMethodFailsOnInvalidServiceName)
        {
            ASSERT_THROW(
                client.FindMethod("invalid.service.name", "Init"),
                ServiceDescriptorNotFoundException
            );
        }

        TEST_F(JsonClientTest, FindMethodFailsOnInvalidMethodName)
        {
            ASSERT_THROW(
                client.FindMethod("nirfsa_grpc.NiRFSA", "InvalidMethodName"),
                MethodDescriptorNotFoundException
            );
        }
    }
}
