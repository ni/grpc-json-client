
#include <chrono>
#include <memory>
#include <string>

#include "gtest/gtest.h"
#include "google/protobuf/descriptor.h"

#include "exceptions.h"
#include "json_client_base.h"
#include "testing_server.h"

using google::protobuf::MethodDescriptor;
using std::chrono::system_clock;
using std::string;
using std::unique_ptr;

namespace ni {
namespace grpc_json_client {

class JsonClientBaseTest : public testing::Test {
 protected:
    JsonClientBase client;
    static unique_ptr<TestingServer> server;
    system_clock::time_point max_deadline;

    JsonClientBaseTest() :
        client("localhost:50051", grpc::InsecureChannelCredentials()),
        max_deadline(system_clock::time_point::max())
    {}

    static void SetUpTestSuite() {
        string address("0.0.0.0:50051");
        server = std::make_unique<TestingServer>(address);
        server->Start();
    }

    static void TearDownTestSuite() {
        server->Stop();
    }

    void SetUp() override {
        client.FillDescriptorDatabase(max_deadline);
    }
};

unique_ptr<TestingServer> JsonClientBaseTest::server;

TEST_F(JsonClientBaseTest, FindMethodSucceedsOnValidMethod) {
    const MethodDescriptor* method;
    string service_name = "ni.grpc_json_client.TestingService";
    string method_name = "UnaryUnaryEcho";
    ASSERT_NO_THROW(method = client.FindMethod(service_name, method_name, max_deadline));
    EXPECT_EQ(method->full_name(), "ni.grpc_json_client.TestingService.UnaryUnaryEcho");
}

TEST_F(JsonClientBaseTest, FindMethodFailsOnUndefinedServiceName) {
    string service_name = "undefined.service.name";
    string method_name = "UnaryUnaryEcho";
    ASSERT_THROW(
        client.FindMethod(service_name, method_name, max_deadline),
        ServiceNotFoundException);
}

TEST_F(JsonClientBaseTest, FindMethodFailsOnUndefinedMethodName) {
    string service_name = "ni.grpc_json_client.TestingService";
    string method_name = "UndefinedMethodName";
    ASSERT_THROW(
        client.FindMethod(service_name, method_name, max_deadline),
        MethodNotFoundException);
}

}  // namespace grpc_json_client
}  // namespace ni
