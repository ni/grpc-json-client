
#include <memory>
#include <string>

#include "gtest/gtest.h"
#include "nlohmann/json.hpp"

#include "error_code.h"
#include "grpc_json_client.h"
#include "testing_server.h"

using nlohmann::json;
using std::string;
using std::unique_ptr;

namespace ni {
namespace grpc_json_client {

class GrpcJsonClientTest : public testing::Test {
 protected:
    static unique_ptr<TestingServer> server;
    void* session = nullptr;

    static void SetUpTestSuite() {
        string address("0.0.0.0:50051");
        server = std::make_unique<TestingServer>(address);
        server->StartInsecure();
    }

    static void TearDownTestSuite() {
server->Stop();
    }

    void SetUp() override {
        ASSERT_EQ(GrpcJsonClient_Initialize("localhost:50051", &session), 0);
        ASSERT_EQ(GrpcJsonClient_QueryReflectionService(session), 0);
    }

    void TearDown() override {
        ASSERT_EQ(GrpcJsonClient_Close(session), 0);
    }
};

unique_ptr<TestingServer> GrpcJsonClientTest::server;

TEST_F(GrpcJsonClientTest, SynchronousCallsWithTimeoutsSucceed) {
    for (string string_field : { "first", "second" }) {
        json request = { {"string_field", string_field} };
        char* service = "ni.grpc_json_client.TestingService";
        void* tag = nullptr;
        int32_t error_code = GrpcJsonClient_StartAsyncCall(
            session, service, "UnaryUnaryEcho", request.dump().c_str(), &tag);
        ASSERT_EQ(error_code, 0);
        size_t size = 0;
        error_code = GrpcJsonClient_FinishAsyncCall(session, tag, 100, nullptr, &size);
        ASSERT_EQ(error_code, 0);
        unique_ptr<char> buffer(new char[size]);
        error_code = GrpcJsonClient_FinishAsyncCall(session, tag, 100, buffer.get(), &size);
        ASSERT_EQ(error_code, 0);
        json response = json::parse(buffer.get());
        ASSERT_EQ(response["string_field"], string_field);
    }
}

TEST_F(GrpcJsonClientTest, SynchronousCallsWithoutTimeoutsSucceed) {
    for (string string_field : { "first", "second" }) {
        json request = { {"delay", 100}, {"string_field", string_field} };
        char* service = "ni.grpc_json_client.TestingService";
        void* tag = nullptr;
        int32_t error_code = GrpcJsonClient_StartAsyncCall(
            session, service, "UnaryUnaryEcho", request.dump().c_str(), &tag);
        ASSERT_EQ(error_code, 0);
        size_t size = 0;
        error_code = GrpcJsonClient_FinishAsyncCall(session, tag, -1, nullptr, &size);
        ASSERT_EQ(error_code, 0);
        unique_ptr<char> buffer(new char[size]);
        error_code = GrpcJsonClient_FinishAsyncCall(session, tag, -1, buffer.get(), &size);
        ASSERT_EQ(error_code, 0);
        json response = json::parse(buffer.get());
        ASSERT_EQ(response["string_field"], string_field);
    }
}

TEST_F(GrpcJsonClientTest, AsynchronousCallsWithTimeoutsSucceed) {
    void* tags[] = { nullptr, nullptr };
    string string_fields[] = { "first", "second" };
    char* service = "ni.grpc_json_client.TestingService";

    for (size_t i = 0; i < 2; i++) {
        json request = { {"string_field", string_fields[i]} };
        int32_t error_code = GrpcJsonClient_StartAsyncCall(
            session, service, "UnaryUnaryEcho", request.dump().c_str(), &tags[i]);
        ASSERT_EQ(error_code, 0);
    }

    for (size_t i = 0; i < 2; i++) {
        size_t size = 0;
        int32_t error_code = GrpcJsonClient_FinishAsyncCall(session, tags[i], 100, nullptr, &size);
        ASSERT_EQ(error_code, 0);
        unique_ptr<char> buffer(new char[size]);
        error_code = GrpcJsonClient_FinishAsyncCall(session, tags[i], 100, buffer.get(), &size);
        ASSERT_EQ(error_code, 0);
        json response = json::parse(buffer.get());
        ASSERT_EQ(response["string_field"], string_fields[i]);
    }
}

TEST_F(GrpcJsonClientTest, AsynchronousCallsWithoutTimeoutsSucceed) {
    void* tags[] = { nullptr, nullptr };
    string string_fields[] = { "first", "second" };
    char* service = "ni.grpc_json_client.TestingService";

    for (size_t i = 0; i < 2; i++) {
        json request = { {"delay", 100}, {"string_field", string_fields[i]} };
        int32_t error_code = GrpcJsonClient_StartAsyncCall(
            session, service, "UnaryUnaryEcho", request.dump().c_str(), &tags[i]);
        ASSERT_EQ(error_code, 0);
    }

    for (size_t i = 0; i < 2; i++) {
        size_t size = 0;
        int32_t error_code = GrpcJsonClient_FinishAsyncCall(session, tags[i], -1, nullptr, &size);
        ASSERT_EQ(error_code, 0);
        unique_ptr<char> buffer(new char[size]);
        error_code = GrpcJsonClient_FinishAsyncCall(session, tags[i], -1, buffer.get(), &size);
        ASSERT_EQ(error_code, 0);
        json response = json::parse(buffer.get());
        ASSERT_EQ(response["string_field"], string_fields[i]);
    }
}

TEST_F(GrpcJsonClientTest, BlockingCallSucceeds) {
    json request = { {"string_field", "BlockingCall"} };
    char* service = "ni.grpc_json_client.TestingService";
    void* tag = nullptr;
    size_t size = 0;
    int32_t error_code = GrpcJsonClient_BlockingCall(
        session, service, "UnaryUnaryEcho", request.dump().c_str(), &tag, 100, nullptr, &size);
    ASSERT_EQ(error_code, 0);
    unique_ptr<char> buffer(new char[size]);
    error_code = GrpcJsonClient_BlockingCall(
        session, nullptr, nullptr, nullptr, &tag, 100, buffer.get(), &size);
    ASSERT_EQ(error_code, 0);
    json response = json::parse(buffer.get());
    ASSERT_EQ(response["string_field"], "BlockingCall");
}

TEST_F(GrpcJsonClientTest, StartAsyncCallToUndefinedServiceFailsWithServiceNotFoundError) {
    void* tag = nullptr;
    int32_t error_code = GrpcJsonClient_StartAsyncCall(
        session, "UndefinedService", "UndefinedMethod", "{}", &tag);
    ASSERT_EQ(error_code, static_cast<int32_t>(ErrorCode::kServiceNotFoundError));
}

TEST_F(GrpcJsonClientTest, StartAsyncCallToUndefinedMethodFailsWithMethodNotFoundError) {
    void* tag = nullptr;
    int32_t error_code = GrpcJsonClient_StartAsyncCall(
        session, "ni.grpc_json_client.TestingService", "UndefinedMethod", "{}", &tag);
    ASSERT_EQ(error_code, static_cast<int32_t>(ErrorCode::kMethodNotFoundError));
}

TEST_F(GrpcJsonClientTest, StartAsyncCallWithMalformedMessageFailsWithSerializationError) {
    void* tag = nullptr;
    int32_t error_code = GrpcJsonClient_StartAsyncCall(
        session, "ni.grpc_json_client.TestingService", "UnaryUnaryEcho", "", &tag);
    ASSERT_EQ(error_code, static_cast<int32_t>(ErrorCode::kSerializationError));
}

TEST_F(GrpcJsonClientTest, FinishAsyncCallWithInvalidTagFailsWithInvalidTagError) {
    void* tag = nullptr;
    int32_t error_code = GrpcJsonClient_StartAsyncCall(
        session, "ni.grpc_json_client.TestingService", "UnaryUnaryEcho", "{}", &tag);
    ASSERT_EQ(error_code, 0);

    size_t size = 0;
    tag = static_cast<int*>(tag) + 1;
    error_code = GrpcJsonClient_FinishAsyncCall(session, tag, 100, nullptr, &size);
    ASSERT_EQ(error_code, static_cast<int32_t>(ErrorCode::kInvalidTagError));
}

TEST_F(GrpcJsonClientTest, CallWithShortTimeoutFailsWithTimeoutError) {
    json request = { {"delay", 100} };
    char* service = "ni.grpc_json_client.TestingService";
    void* tag = nullptr;
    int32_t error_code = GrpcJsonClient_StartAsyncCall(
        session, service, "UnaryUnaryEcho", request.dump().c_str(), &tag);
    ASSERT_EQ(error_code, 0);

    size_t size = 0;
    error_code = GrpcJsonClient_FinishAsyncCall(session, tag, 10, nullptr, &size);
    ASSERT_EQ(error_code, static_cast<int32_t>(ErrorCode::kTimeoutError));
}

TEST_F(GrpcJsonClientTest, FinishAsyncCallWithSmallBufferFailsWithBufferSizeOutOfRangeError) {
    void* tag = nullptr;
    int32_t error_code = GrpcJsonClient_StartAsyncCall(
        session, "ni.grpc_json_client.TestingService", "UnaryUnaryEcho", "{}", &tag);
    ASSERT_EQ(error_code, 0);

    size_t size = 0;
    char* buffer = reinterpret_cast<char*>(1);
    error_code = GrpcJsonClient_FinishAsyncCall(session, tag, 100, buffer, &size);
    ASSERT_EQ(error_code, static_cast<int32_t>(ErrorCode::kBufferSizeOutOfRangeError));
}

TEST_F(GrpcJsonClientTest, GetErrorSucceeds) {
    void* tag = nullptr;
    int32_t expected_error_code = GrpcJsonClient_StartAsyncCall(
        session, "UndefinedService", "UndefinedMethod", "{}", &tag);

    int32_t queried_error_code = 0;
    size_t size = 0;
    int32_t get_error_error_code = GrpcJsonClient_GetError(
        session, &queried_error_code, nullptr, &size);
    ASSERT_EQ(get_error_error_code, 0);
    unique_ptr<char> buffer(new char[size]);
    get_error_error_code = GrpcJsonClient_GetError(
        session, &queried_error_code, buffer.get(), &size);
    ASSERT_EQ(get_error_error_code, 0);

    ASSERT_EQ(queried_error_code, expected_error_code);  // returns most recent code
}

TEST_F(GrpcJsonClientTest, GetErrorStringWithoutSessionSucceeds) {
    void* tag = nullptr;
    int32_t error_code = GrpcJsonClient_StartAsyncCall(
        session, "UndefinedService", "UndefinedMethod", "{}", &tag);

    size_t size = 0;
    int32_t get_error_error_code = GrpcJsonClient_GetErrorString(nullptr, error_code, nullptr, &size);
    ASSERT_EQ(get_error_error_code, 0);
    unique_ptr<char> buffer(new char[size]);
    get_error_error_code = GrpcJsonClient_GetErrorString(nullptr, error_code, buffer.get(), &size);
    ASSERT_EQ(get_error_error_code, 0);

    string expected_description = GetErrorString(static_cast<ErrorCode>(error_code));
    ASSERT_STREQ(buffer.get(), expected_description.c_str());
}

TEST_F(GrpcJsonClientTest, GetErrorWithSmallBufferSucceedsWithBufferSizeOutOfRangeWarning) {
    void* tag = nullptr;
    GrpcJsonClient_StartAsyncCall(session, "UndefinedService", "UndefinedMethod", "{}", &tag);

    int32_t queried_error_code = 0;
    char buffer = 0;
    size_t size = 1;
    int32_t error_code = GrpcJsonClient_GetError(session, &queried_error_code, &buffer, &size);

    ASSERT_EQ(error_code, static_cast<int32_t>(ErrorCode::kBufferSizeOutOfRangeWarning));
}

}  // namespace grpc_json_client
}  // namespace ni
