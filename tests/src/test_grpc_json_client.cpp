
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
    intptr_t session;

    static void SetUpTestSuite() {
        string address("0.0.0.0:50051");
        server = std::make_unique<TestingServer>(address);
        server->Start();
    }

    static void TearDownTestSuite() {
        server->Stop();
    }

    void SetUp() override {
        ASSERT_EQ(GrpcJsonClient_Initialize("localhost:50051", &session), 0);
    }

    void TearDown() override {
        ASSERT_EQ(GrpcJsonClient_Close(session), 0);
    }
};

unique_ptr<TestingServer> GrpcJsonClientTest::server;

TEST_F(GrpcJsonClientTest, ResetDescriptorDatabaseSucceeds) {
    ASSERT_EQ(GrpcJsonClient_ResetDescriptorDatabase(session), 0);
}

TEST_F(GrpcJsonClientTest, FillDescriptorDatabaseSucceeds) {
    ASSERT_EQ(GrpcJsonClient_FillDescriptorDatabase(session, -1), 0);
}

TEST_F(GrpcJsonClientTest, SynchronousCallsWithTimeoutsSucceed) {
    for (string string_field : { "first", "second" }) {
        json request = { {"string_field", string_field} };
        char* service = "ni.grpc_json_client.TestingService";
        intptr_t tag = 0;
        int32_t error_code = GrpcJsonClient_StartAsyncCall(
            session, service, "UnaryUnaryEcho", request.dump().c_str(), 100, &tag);
        ASSERT_EQ(error_code, 0);
        size_t size = 0;
        error_code = GrpcJsonClient_FinishAsyncCall(session, tag, 100, 0, &size);
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
        intptr_t tag = 0;
        int32_t error_code = GrpcJsonClient_StartAsyncCall(
            session, service, "UnaryUnaryEcho", request.dump().c_str(), -1, &tag);
        ASSERT_EQ(error_code, 0);
        size_t size = 0;
        error_code = GrpcJsonClient_FinishAsyncCall(session, tag, -1, 0, &size);
        ASSERT_EQ(error_code, 0);
        unique_ptr<char> buffer(new char[size]);
        error_code = GrpcJsonClient_FinishAsyncCall(session, tag, -1, buffer.get(), &size);
        ASSERT_EQ(error_code, 0);
        json response = json::parse(buffer.get());
        ASSERT_EQ(response["string_field"], string_field);
    }
}

TEST_F(GrpcJsonClientTest, AsynchronousCallsWithTimeoutsSucceed) {
    intptr_t tags[] = { 0, 0 };
    string string_fields[] = { "first", "second" };
    char* service = "ni.grpc_json_client.TestingService";

    for (size_t i = 0; i < 2; i++) {
        json request = { {"string_field", string_fields[i]} };
        int32_t error_code = GrpcJsonClient_StartAsyncCall(
            session, service, "UnaryUnaryEcho", request.dump().c_str(), 100, &tags[i]);
        ASSERT_EQ(error_code, 0);
    }

    for (size_t i = 0; i < 2; i++) {
        size_t size = 0;
        int32_t error_code = GrpcJsonClient_FinishAsyncCall(session, tags[i], 100, 0, &size);
        ASSERT_EQ(error_code, 0);
        unique_ptr<char> buffer(new char[size]);
        error_code = GrpcJsonClient_FinishAsyncCall(session, tags[i], 100, buffer.get(), &size);
        ASSERT_EQ(error_code, 0);
        json response = json::parse(buffer.get());
        ASSERT_EQ(response["string_field"], string_fields[i]);
    }
}

TEST_F(GrpcJsonClientTest, AsynchronousCallsWithoutTimeoutsSucceed) {
    intptr_t tags[] = { 0, 0 };
    string string_fields[] = { "first", "second" };
    char* service = "ni.grpc_json_client.TestingService";

    for (size_t i = 0; i < 2; i++) {
        json request = { {"delay", 100}, {"string_field", string_fields[i]} };
        int32_t error_code = GrpcJsonClient_StartAsyncCall(
            session, service, "UnaryUnaryEcho", request.dump().c_str(), -1, &tags[i]);
        ASSERT_EQ(error_code, 0);
    }

    for (size_t i = 0; i < 2; i++) {
        size_t size = 0;
        int32_t error_code = GrpcJsonClient_FinishAsyncCall(session, tags[i], -1, 0, &size);
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
    intptr_t tag = 0;
    size_t size = 0;
    int32_t error_code = GrpcJsonClient_BlockingCall(
        session, service, "UnaryUnaryEcho", request.dump().c_str(), 100, &tag, 0, &size);
    ASSERT_EQ(error_code, 0);
    unique_ptr<char> buffer(new char[size]);
    error_code = GrpcJsonClient_BlockingCall(session, 0, 0, 0, 100, &tag, buffer.get(), &size);
    ASSERT_EQ(error_code, 0);
    json response = json::parse(buffer.get());
    ASSERT_EQ(response["string_field"], "BlockingCall");
}

TEST_F(GrpcJsonClientTest, StartAsyncCallToUndefinedServiceFailsWithServiceNotFoundError) {
    intptr_t tag = 0;
    int32_t error_code = GrpcJsonClient_StartAsyncCall(
        session, "UndefinedService", "UndefinedMethod", "{}", -1, &tag);
    ASSERT_EQ(error_code, static_cast<int32_t>(ErrorCode::kServiceNotFoundError));
}

TEST_F(GrpcJsonClientTest, StartAsyncCallToUndefinedMethodFailsWithMethodNotFoundError) {
    intptr_t tag = 0;
    int32_t error_code = GrpcJsonClient_StartAsyncCall(
        session, "ni.grpc_json_client.TestingService", "UndefinedMethod", "{}", -1, &tag);
    ASSERT_EQ(error_code, static_cast<int32_t>(ErrorCode::kMethodNotFoundError));
}

TEST_F(GrpcJsonClientTest, StartAsyncCallWithMalformedMessageFailsWithSerializationError) {
    intptr_t tag = 0;
    int32_t error_code = GrpcJsonClient_StartAsyncCall(
        session, "ni.grpc_json_client.TestingService", "UnaryUnaryEcho", "", -1, &tag);
    ASSERT_EQ(error_code, static_cast<int32_t>(ErrorCode::kSerializationError));
}

TEST_F(GrpcJsonClientTest, FinishAsyncCallWithInvalidTagFailsWithInvalidTagError) {
    intptr_t tag = 0;
    int32_t error_code = GrpcJsonClient_StartAsyncCall(
        session, "ni.grpc_json_client.TestingService", "UnaryUnaryEcho", "{}", -1, &tag);
    ASSERT_EQ(error_code, 0);

    size_t size = 0;
    tag++;
    error_code = GrpcJsonClient_FinishAsyncCall(session, tag, 100, 0, &size);
    ASSERT_EQ(error_code, static_cast<int32_t>(ErrorCode::kInvalidArgumentError));
}

TEST_F(GrpcJsonClientTest, CallWithShortTimeoutFailsWithTimeoutError) {
    json request = { {"delay", 100} };
    char* service = "ni.grpc_json_client.TestingService";
    intptr_t tag = 0;
    int32_t error_code = GrpcJsonClient_StartAsyncCall(
        session, service, "UnaryUnaryEcho", request.dump().c_str(), -1, &tag);
    ASSERT_EQ(error_code, 0);

    size_t size = 0;
    error_code = GrpcJsonClient_FinishAsyncCall(session, tag, 10, 0, &size);
    ASSERT_EQ(error_code, static_cast<int32_t>(ErrorCode::kTimeoutError));
}

TEST_F(GrpcJsonClientTest, FinishAsyncCallWithSmallBufferFailsWithBufferSizeOutOfRangeError) {
    intptr_t tag = 0;
    int32_t error_code = GrpcJsonClient_StartAsyncCall(
        session, "ni.grpc_json_client.TestingService", "UnaryUnaryEcho", "{}", -1, &tag);
    ASSERT_EQ(error_code, 0);

    size_t size = 0;
    char* buffer = reinterpret_cast<char*>(1);
    error_code = GrpcJsonClient_FinishAsyncCall(session, tag, 100, buffer, &size);
    ASSERT_EQ(error_code, static_cast<int32_t>(ErrorCode::kBufferSizeOutOfRangeError));
}

TEST_F(GrpcJsonClientTest, GetErrorSucceeds) {
    intptr_t tag = 0;
    int32_t expected_error_code = GrpcJsonClient_StartAsyncCall(
        session, "UndefinedService", "UndefinedMethod", "{}", -1, &tag);

    int32_t queried_error_code = 0;
    size_t size = 0;
    int32_t get_error_error_code = GrpcJsonClient_GetError(
        session, &queried_error_code, 0, &size);
    ASSERT_EQ(get_error_error_code, 0);
    unique_ptr<char> buffer(new char[size]);
    get_error_error_code = GrpcJsonClient_GetError(
        session, &queried_error_code, buffer.get(), &size);
    ASSERT_EQ(get_error_error_code, 0);

    ASSERT_EQ(queried_error_code, expected_error_code);  // returns most recent code
}

TEST_F(GrpcJsonClientTest, GetErrorStringWithoutSessionSucceeds) {
    intptr_t tag = 0;
    int32_t error_code = GrpcJsonClient_StartAsyncCall(
        session, "UndefinedService", "UndefinedMethod", "{}", -1, &tag);

    size_t size = 0;
    int32_t get_error_error_code = {
        GrpcJsonClient_GetErrorString(0, error_code, 0, &size)
    };
    ASSERT_EQ(get_error_error_code, 0);
    unique_ptr<char> buffer(new char[size]);
    get_error_error_code = GrpcJsonClient_GetErrorString(0, error_code, buffer.get(), &size);
    ASSERT_EQ(get_error_error_code, 0);

    string expected_description = GetErrorString(static_cast<ErrorCode>(error_code));
    ASSERT_STREQ(buffer.get(), expected_description.c_str());
}

TEST_F(GrpcJsonClientTest, GetErrorWithSmallBufferSucceedsWithBufferSizeOutOfRangeWarning) {
    intptr_t tag = 0;
    GrpcJsonClient_StartAsyncCall(session, "UndefinedService", "UndefinedMethod", "{}", -1, &tag);

    int32_t queried_error_code = 0;
    char buffer = 0;
    size_t size = 1;
    int32_t error_code = GrpcJsonClient_GetError(session, &queried_error_code, &buffer, &size);

    ASSERT_EQ(error_code, static_cast<int32_t>(ErrorCode::kBufferSizeOutOfRangeWarning));
}

TEST_F(GrpcJsonClientTest, RequestsWithoutRunningServiceFailWithRemoteProcedureCallError) {
    server->Stop();
    EXPECT_EQ(GrpcJsonClient_FillDescriptorDatabase(session, -1), -2);
    server->Start();
}

}  // namespace grpc_json_client
}  // namespace ni
