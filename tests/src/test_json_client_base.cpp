
#include <memory>
#include <string>

#include "gtest/gtest.h"
#include "google/protobuf/descriptor.h"

#include "exceptions.h"
#include "json_client_base.h"
#include "testing_server.h"

using google::protobuf::MethodDescriptor;
using std::string;
using std::unique_ptr;

namespace ni {
namespace grpc_json_client {

class JsonClientBaseTest : public testing::Test {
 protected:
    JsonClientBase client;
    static unique_ptr<TestingServer> server;

    JsonClientBaseTest() :
        client("localhost:50051", grpc::InsecureChannelCredentials())
    {}

    static void SetUpTestSuite() {
        string address("0.0.0.0:50051");
        server = std::make_unique<TestingServer>(address);
        server->StartInsecure();
    }

    static void TearDownTestSuite() {
        server->Stop();
    }

    void SetUp() override {
        client.QueryReflectionService();
    }
};

unique_ptr<TestingServer> JsonClientBaseTest::server;

TEST_F(JsonClientBaseTest, FindMethodSucceedsOnValidMethod) {
    const MethodDescriptor* method;
    ASSERT_NO_THROW(
        method = client.FindMethod("ni.grpc_json_client.TestingService", "UnaryUnaryEcho"));
    EXPECT_EQ(method->full_name(), "ni.grpc_json_client.TestingService.UnaryUnaryEcho");
}

TEST_F(JsonClientBaseTest, FindMethodFailsOnUndefinedServiceName) {
    ASSERT_THROW(
        client.FindMethod("undefined.service.name", "UnaryUnaryEcho"),
        ServiceDescriptorNotFoundException);
}

TEST_F(JsonClientBaseTest, FindMethodFailsOnUndefinedMethodName) {
    ASSERT_THROW(
        client.FindMethod("ni.grpc_json_client.TestingService", "UndefinedMethodName"),
        MethodDescriptorNotFoundException);
}

}  // namespace grpc_json_client
}  // namespace ni
