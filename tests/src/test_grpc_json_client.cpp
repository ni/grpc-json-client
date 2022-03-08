
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

class GrpcJsonClientOfflineTest : public testing::Test {
 protected:
    intptr_t session;

    GrpcJsonClientOfflineTest() :
        session(0)
    {}

    void SetUp() override {
        ASSERT_EQ(GrpcJsonClient_Initialize("localhost:50051", &session), 0);
    }

    void TearDown() override {
        ASSERT_EQ(GrpcJsonClient_Close(session), 0);
    }
};

class GrpcJsonClientTest : public GrpcJsonClientOfflineTest {
 protected:
    static unique_ptr<TestingServer> server;
    static const char* testing_service;

    static void SetUpTestSuite() {
        string address("0.0.0.0:50051");
        server = std::make_unique<TestingServer>(address);
        server->Start();
    }

    static void TearDownTestSuite() {
        server->Stop();
    }
};

unique_ptr<TestingServer> GrpcJsonClientTest::server;
const char* GrpcJsonClientTest::testing_service = "ni.grpc_json_client.TestingService";

int32_t GetErrorHelper(intptr_t session, int32_t* code, string* message) {
    size_t size = 0;
    int32_t error_code = GrpcJsonClient_GetError(session, nullptr, nullptr, &size);
    if (error_code < 0) {
        return error_code;
    }
    unique_ptr<char> buffer(new char[size]);
    int32_t next_error_code = GrpcJsonClient_GetError(session, code, buffer.get(), &size);
    if (next_error_code < 0) {
        return next_error_code;
    }
    *message = buffer.get();
    return error_code > 0 ? error_code : next_error_code;
}

TEST_F(GrpcJsonClientOfflineTest, ResetDescriptorDatabaseSucceeds) {
    ASSERT_EQ(GrpcJsonClient_ResetDescriptorDatabase(session), 0);
}

TEST_F(GrpcJsonClientOfflineTest, FillDescriptorDatabaseFailsWithRemoteProcedureCallError) {
    int32_t expected_code = static_cast<int32_t>(ErrorCode::kRemoteProcedureCallError);
    ASSERT_EQ(GrpcJsonClient_FillDescriptorDatabase(session, -1), expected_code);
}

TEST_F(GrpcJsonClientOfflineTest, StartAsyncCallFailsWithRemoteProcedureCallError) {
    int32_t expected_code = static_cast<int32_t>(ErrorCode::kRemoteProcedureCallError);
    ASSERT_EQ(GrpcJsonClient_StartAsyncCall(session, "", "", "", -1, 0), expected_code);
}

TEST_F(GrpcJsonClientOfflineTest, FinishAsyncCallFailsWithInvalidTagError) {
    int32_t expected_code = static_cast<int32_t>(ErrorCode::kInvalidArgumentError);
    ASSERT_EQ(GrpcJsonClient_FinishAsyncCall(session, 0, -1, nullptr, nullptr), expected_code);
}

TEST_F(GrpcJsonClientOfflineTest, BlockingCallFailsWithRemoteProcedureCallError) {
    intptr_t tag = 0;
    int32_t error_code = GrpcJsonClient_BlockingCall(
        session, "", "", "", -1, &tag, nullptr, nullptr);
    int32_t expected_code = static_cast<int32_t>(ErrorCode::kRemoteProcedureCallError);
    ASSERT_EQ(error_code, expected_code);
}

TEST_F(GrpcJsonClientOfflineTest, LockAndUnlockSessionSucceeds) {
    ASSERT_EQ(GrpcJsonClient_LockSession(session), 0);
    ASSERT_EQ(GrpcJsonClient_UnlockSession(session), 0);
}

TEST_F(GrpcJsonClientOfflineTest, RecursiveLockingSucceeds) {
    ASSERT_EQ(GrpcJsonClient_LockSession(session), 0);
    ASSERT_EQ(GrpcJsonClient_LockSession(session), 0);
    ASSERT_EQ(GrpcJsonClient_UnlockSession(session), 0);
    ASSERT_EQ(GrpcJsonClient_UnlockSession(session), 0);
}

TEST_F(GrpcJsonClientOfflineTest, GetErrorWithNoErrorSucceeds) {
    int32_t code = 0;
    string message;
    ASSERT_EQ(GetErrorHelper(session, &code, &message), 0);
    ASSERT_EQ(code, 0);
    ASSERT_EQ(message, "No error");
}

TEST_F(GrpcJsonClientOfflineTest, GetErrorStringWithNoErrorSucceeds) {
    int32_t code = 0;
    size_t size = 0;
    ASSERT_EQ(GrpcJsonClient_GetErrorString(session, code, nullptr, &size), 0);
    unique_ptr<char> buffer(new char[size]);
    ASSERT_EQ(GrpcJsonClient_GetErrorString(session, code, buffer.get(), &size), 0);
    ASSERT_EQ(code, 0);
    ASSERT_STREQ(buffer.get(), "No error");
}

TEST_F(GrpcJsonClientOfflineTest, GetErrorStringWithErrorSucceeds) {
    int32_t code = -1;
    size_t size = 0;
    ASSERT_EQ(GrpcJsonClient_GetErrorString(session, code, nullptr, &size), 0);
    unique_ptr<char> buffer(new char[size]);
    ASSERT_EQ(GrpcJsonClient_GetErrorString(session, code, buffer.get(), &size), 0);
    ASSERT_STREQ(buffer.get(), "Unknown error");
}

TEST_F(
    GrpcJsonClientOfflineTest,
    GetErrorStringWithOutOfRangeErrorCodeReturnsUndefinedErrorCodeMessage
) {
    int32_t code = INT32_MIN;
    size_t size = 0;
    ASSERT_EQ(GrpcJsonClient_GetErrorString(session, code, nullptr, &size), 0);
    unique_ptr<char> buffer(new char[size]);
    ASSERT_EQ(GrpcJsonClient_GetErrorString(session, code, buffer.get(), &size), 0);
    ASSERT_STREQ(buffer.get(), "Undefined error code");
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

}  // namespace grpc_json_client
}  // namespace ni
