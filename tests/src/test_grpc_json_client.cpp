
#include <memory>
#include <string>

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <nlohmann/json.hpp>

#include "common.h"
#include "error_code.h"
#include "grpc_json_client.h"

#include "testing_server.h"

using nlohmann::json;
using std::string;
using std::unique_ptr;

namespace ni {
namespace grpc_json_client {

class GrpcJsonClientServerOfflineTest : public testing::Test {
 protected:
    intptr_t session;

    GrpcJsonClientServerOfflineTest() :
        session(0)
    {}

    void SetUp() override {
        ASSERT_EQ(GrpcJsonClient_Initialize("localhost:50051", &session), 0);
    }

    void TearDown() override {
        ASSERT_EQ(GrpcJsonClient_Close(session), 0);
    }
};

class GrpcJsonClientTest : public GrpcJsonClientServerOfflineTest {
 private:
    static unique_ptr<TestingServer> server;

 protected:
    static const char* testing_service;
    static const char* address;

    static void SetUpTestSuite() {
        server = std::make_unique<TestingServer>(address);
        server->EnableReflection();
        server->Start();
    }

    static void TearDownTestSuite() {
        server->Stop();
    }
};

class GrpcJsonClientNoServerReflectionTest : public GrpcJsonClientTest {
 private:
    static unique_ptr<TestingServer> server;

 protected:
    static void SetUpTestSuite() {
        server = std::make_unique<TestingServer>(address);
        server->Start();
    }

    static void TearDownTestSuite() {
        server->Stop();
    }
};

unique_ptr<TestingServer> GrpcJsonClientTest::server;
const char* GrpcJsonClientTest::testing_service = "ni.grpc_json_client.TestingService";
const char* GrpcJsonClientTest::address = "0.0.0.0:50051";
unique_ptr<TestingServer> GrpcJsonClientNoServerReflectionTest::server;

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

TEST_F(GrpcJsonClientServerOfflineTest, ResetDescriptorDatabaseSucceeds) {
    int32_t error_code = GrpcJsonClient_ResetDescriptorDatabase(session);
    int32_t expected_error_code = static_cast<int32_t>(ErrorCode::kNone);
    ASSERT_EQ(error_code, expected_error_code);
}

TEST_F(GrpcJsonClientServerOfflineTest, FillDescriptorDatabaseFailsWithRemoteProcedureCallError) {
    int32_t error_code = GrpcJsonClient_FillDescriptorDatabase(session, -1);
    int32_t expected_code = static_cast<int32_t>(ErrorCode::kRemoteProcedureCallError);
    ASSERT_EQ(error_code, expected_code);

    int32_t queried_code;
    string message;
    GetErrorHelper(session, &queried_code, &message);
    ASSERT_THAT(message, testing::HasSubstr("Failed to initiate communication with the host."));
}

TEST_F(GrpcJsonClientServerOfflineTest, StartAsyncCallFailsWithRemoteProcedureCallError) {
    int32_t error_code = GrpcJsonClient_StartAsyncCall(session, "", "", "", -1, 0);
    int32_t expected_code = static_cast<int32_t>(ErrorCode::kRemoteProcedureCallError);
    ASSERT_EQ(error_code, expected_code);
}

TEST_F(GrpcJsonClientServerOfflineTest, FinishAsyncCallFailsWithInvalidTagError) {
    int32_t error_code = GrpcJsonClient_FinishAsyncCall(session, 0, -1, nullptr, nullptr);
    int32_t expected_code = static_cast<int32_t>(ErrorCode::kInvalidArgumentError);
    ASSERT_EQ(error_code, expected_code);
}

TEST_F(GrpcJsonClientServerOfflineTest, BlockingCallFailsWithRemoteProcedureCallError) {
    intptr_t tag = 0;
    int32_t error_code = GrpcJsonClient_BlockingCall(
        session, "", "", "", -1, &tag, nullptr, nullptr);
    int32_t expected_code = static_cast<int32_t>(ErrorCode::kRemoteProcedureCallError);
    ASSERT_EQ(error_code, expected_code);
}

TEST_F(GrpcJsonClientServerOfflineTest, LockAndUnlockSessionSucceeds) {
    uint8_t has_lock = 0;
    int32_t error_code = GrpcJsonClient_LockSession(session, -1, &has_lock);
    int32_t expected_code = static_cast<int32_t>(ErrorCode::kNone);
    ASSERT_EQ(error_code, expected_code);
    ASSERT_EQ(has_lock, 1);
    error_code = GrpcJsonClient_UnlockSession(session);
    ASSERT_EQ(error_code, expected_code);
}

TEST_F(GrpcJsonClientServerOfflineTest, RecursiveLockingSucceeds) {
    uint8_t has_lock = 0;
    int32_t error_code = GrpcJsonClient_LockSession(session, -1, &has_lock);
    int32_t expected_code = static_cast<int32_t>(ErrorCode::kNone);
    ASSERT_EQ(error_code, expected_code);
    ASSERT_EQ(has_lock, 1);
    error_code = GrpcJsonClient_LockSession(session, -1, &has_lock);
    ASSERT_EQ(error_code, expected_code);
    ASSERT_EQ(has_lock, 1);
    error_code = GrpcJsonClient_UnlockSession(session);
    ASSERT_EQ(error_code, expected_code);
    error_code = GrpcJsonClient_UnlockSession(session);
    ASSERT_EQ(error_code, expected_code);
}

TEST_F(GrpcJsonClientServerOfflineTest, GetErrorWithNoErrorSucceeds) {
    int32_t queried_code = 0;
    string message;
    int32_t error_code = GetErrorHelper(session, &queried_code, &message);
    int32_t expected_code = static_cast<int32_t>(ErrorCode::kNone);
    ASSERT_EQ(error_code, expected_code);
    ASSERT_EQ(queried_code, expected_code);
    ASSERT_EQ(message, "No error");
}

TEST_F(GrpcJsonClientServerOfflineTest, GetErrorStringWithNoErrorSucceeds) {
    int32_t requested_code = 0;
    size_t size = 0;
    int32_t error_code = GrpcJsonClient_GetErrorString(session, requested_code, nullptr, &size);
    int32_t expected_code = static_cast<int32_t>(ErrorCode::kNone);
    ASSERT_EQ(error_code, expected_code);
    unique_ptr<char> buffer(new char[size]);
    error_code = GrpcJsonClient_GetErrorString(session, requested_code, buffer.get(), &size);
    ASSERT_EQ(error_code, expected_code);
    ASSERT_EQ(requested_code, expected_code);
    ASSERT_STREQ(buffer.get(), "No error");
}

TEST_F(GrpcJsonClientServerOfflineTest, GetErrorStringWithErrorSucceeds) {
    int32_t requested_code = -1;
    size_t size = 0;
    int32_t error_code = GrpcJsonClient_GetErrorString(session, requested_code, nullptr, &size);
    int32_t expected_code = static_cast<int32_t>(ErrorCode::kNone);
    ASSERT_EQ(error_code, expected_code);
    unique_ptr<char> buffer(new char[size]);
    error_code = GrpcJsonClient_GetErrorString(session, requested_code, buffer.get(), &size);
    ASSERT_EQ(error_code, expected_code);
    ASSERT_STREQ(buffer.get(), "Unknown error");
}

TEST_F(
    GrpcJsonClientServerOfflineTest,
    GetErrorStringWithOutOfRangeErrorCodeReturnsUndefinedErrorCodeMessage
) {
    int32_t requested_code = INT32_MIN;
    size_t size = 0;
    int32_t error_code = GrpcJsonClient_GetErrorString(session, requested_code, nullptr, &size);
    int32_t expected_code = static_cast<int32_t>(ErrorCode::kNone);
    ASSERT_EQ(error_code, expected_code);
    unique_ptr<char> buffer(new char[size]);
    error_code = GrpcJsonClient_GetErrorString(session, requested_code, buffer.get(), &size);
    ASSERT_EQ(error_code, expected_code);
    ASSERT_STREQ(buffer.get(), "Undefined error code");
}

TEST_F(
    GrpcJsonClientServerOfflineTest,
    GetErrorStringWithSmallBufferSucceedsWithBufferSizeOutOfRangeWarning
) {
    char buffer = 0;
    size_t size = 1;
    int32_t error_code = GrpcJsonClient_GetErrorString(session, 0, &buffer, &size);
    int32_t expected_code = static_cast<int32_t>(ErrorCode::kBufferSizeOutOfRangeWarning);
    ASSERT_EQ(error_code, expected_code);
}

TEST_F(GrpcJsonClientServerOfflineTest, GetErrorStringWithSmallBufferPopulatesErrorState) {
    char buffer = 0;
    size_t size = 1;
    int32_t error_code = GrpcJsonClient_GetErrorString(session, 0, &buffer, &size);
    int32_t expected_code = static_cast<int32_t>(ErrorCode::kBufferSizeOutOfRangeWarning);
    ASSERT_EQ(error_code, expected_code);

    int32_t queried_error_code;
    string message;
    error_code = GetErrorHelper(session, &queried_error_code, &message);
    ASSERT_EQ(queried_error_code, static_cast<int32_t>(ErrorCode::kBufferSizeOutOfRangeWarning));
    expected_code = static_cast<int32_t>(ErrorCode::kNone);
    ASSERT_EQ(error_code, expected_code);
}

TEST_F(GrpcJsonClientServerOfflineTest, GetErrorClearsErrorState) {
    char buffer = 0;
    size_t size = 1;
    int32_t error_code = GrpcJsonClient_GetErrorString(session, 0, &buffer, &size);
    int32_t expected_code = static_cast<int32_t>(ErrorCode::kBufferSizeOutOfRangeWarning);
    ASSERT_EQ(error_code, expected_code);

    int32_t queried_error_code;
    string message;
    GetErrorHelper(session, &queried_error_code, &message);
    GetErrorHelper(session, &queried_error_code, &message);

    expected_code = static_cast<int32_t>(ErrorCode::kNone);
    ASSERT_EQ(queried_error_code, expected_code);
    ASSERT_EQ(message, "No error");
}

TEST_F(GrpcJsonClientTest, FillDescriptorDatabaseSucceeds) {
    int32_t error_code = GrpcJsonClient_FillDescriptorDatabase(session, -1);
    int32_t expected_code = static_cast<int32_t>(ErrorCode::kNone);
    ASSERT_EQ(error_code, expected_code);
}

TEST_F(GrpcJsonClientTest, SynchronousCallsWithTimeoutsSucceed) {
    for (string string_field : { "first", "second" }) {
        json request = { {"string_field", string_field} };
        char* service = "ni.grpc_json_client.TestingService";
        intptr_t tag = 0;
        int32_t error_code = GrpcJsonClient_StartAsyncCall(
            session, service, "UnaryUnaryEcho", request.dump().c_str(), 100, &tag);
        int32_t expected_code = static_cast<int32_t>(ErrorCode::kNone);
        ASSERT_EQ(error_code, expected_code);
        size_t size = 0;
        error_code = GrpcJsonClient_FinishAsyncCall(session, tag, 100, 0, &size);
        ASSERT_EQ(error_code, expected_code);
        unique_ptr<char> buffer(new char[size]);
        error_code = GrpcJsonClient_FinishAsyncCall(session, tag, 100, buffer.get(), &size);
        ASSERT_EQ(error_code, expected_code);
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
        int32_t expected_code = static_cast<int32_t>(ErrorCode::kNone);
        ASSERT_EQ(error_code, expected_code);
        size_t size = 0;
        error_code = GrpcJsonClient_FinishAsyncCall(session, tag, -1, 0, &size);
        ASSERT_EQ(error_code, expected_code);
        unique_ptr<char> buffer(new char[size]);
        error_code = GrpcJsonClient_FinishAsyncCall(session, tag, -1, buffer.get(), &size);
        ASSERT_EQ(error_code, expected_code);
        json response = json::parse(buffer.get());
        ASSERT_EQ(response["string_field"], string_field);
    }
}

TEST_F(GrpcJsonClientTest, AsynchronousCallsWithTimeoutsSucceed) {
    intptr_t tags[] = { 0, 0 };
    string string_fields[] = { "first", "second" };
    char* service = "ni.grpc_json_client.TestingService";

    int32_t expected_code = static_cast<int32_t>(ErrorCode::kNone);
    for (size_t i = 0; i < 2; i++) {
        json request = { {"string_field", string_fields[i]} };
        int32_t error_code = GrpcJsonClient_StartAsyncCall(
            session, service, "UnaryUnaryEcho", request.dump().c_str(), 100, &tags[i]);
        ASSERT_EQ(error_code, expected_code);
    }

    for (size_t i = 0; i < 2; i++) {
        size_t size = 0;
        int32_t error_code = GrpcJsonClient_FinishAsyncCall(session, tags[i], 100, 0, &size);
        ASSERT_EQ(error_code, expected_code);
        unique_ptr<char> buffer(new char[size]);
        error_code = GrpcJsonClient_FinishAsyncCall(session, tags[i], 100, buffer.get(), &size);
        ASSERT_EQ(error_code, expected_code);
        json response = json::parse(buffer.get());
        ASSERT_EQ(response["string_field"], string_fields[i]);
    }
}

TEST_F(GrpcJsonClientTest, AsynchronousCallsWithoutTimeoutsSucceed) {
    intptr_t tags[] = { 0, 0 };
    string string_fields[] = { "first", "second" };
    char* service = "ni.grpc_json_client.TestingService";

    int32_t expected_code = static_cast<int32_t>(ErrorCode::kNone);
    for (size_t i = 0; i < 2; i++) {
        json request = { {"delay", 100}, {"string_field", string_fields[i]} };
        int32_t error_code = GrpcJsonClient_StartAsyncCall(
            session, service, "UnaryUnaryEcho", request.dump().c_str(), -1, &tags[i]);
        ASSERT_EQ(error_code, 0);
    }

    for (size_t i = 0; i < 2; i++) {
        size_t size = 0;
        int32_t error_code = GrpcJsonClient_FinishAsyncCall(session, tags[i], -1, 0, &size);
        ASSERT_EQ(error_code, expected_code);
        unique_ptr<char> buffer(new char[size]);
        error_code = GrpcJsonClient_FinishAsyncCall(session, tags[i], -1, buffer.get(), &size);
        ASSERT_EQ(error_code, expected_code);
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
    int32_t expected_code = static_cast<int32_t>(ErrorCode::kNone);
    ASSERT_EQ(error_code, expected_code);
    unique_ptr<char> buffer(new char[size]);
    error_code = GrpcJsonClient_BlockingCall(session, 0, 0, 0, 100, &tag, buffer.get(), &size);
    ASSERT_EQ(error_code, expected_code);
    json response = json::parse(buffer.get());
    ASSERT_EQ(response["string_field"], "BlockingCall");
}

TEST_F(GrpcJsonClientTest, StartAsyncCallToUndefinedServiceFailsWithServiceNotFoundError) {
    intptr_t tag = 0;
    int32_t error_code = GrpcJsonClient_StartAsyncCall(
        session, "UndefinedService", "UndefinedMethod", "{}", -1, &tag);
    int32_t expected_code = static_cast<int32_t>(ErrorCode::kServiceNotFoundError);
    ASSERT_EQ(error_code, expected_code);
}

TEST_F(GrpcJsonClientTest, StartAsyncCallToUndefinedMethodFailsWithMethodNotFoundError) {
    intptr_t tag = 0;
    int32_t error_code = GrpcJsonClient_StartAsyncCall(
        session, "ni.grpc_json_client.TestingService", "UndefinedMethod", "{}", -1, &tag);
    int32_t expected_code = static_cast<int32_t>(ErrorCode::kMethodNotFoundError);
    ASSERT_EQ(error_code, expected_code);
}

TEST_F(GrpcJsonClientTest, StartAsyncCallWithMalformedMessageFailsWithSerializationError) {
    intptr_t tag = 0;
    int32_t error_code = GrpcJsonClient_StartAsyncCall(
        session, "ni.grpc_json_client.TestingService", "UnaryUnaryEcho", "", -1, &tag);
    int32_t expected_code = static_cast<int32_t>(ErrorCode::kSerializationError);
    ASSERT_EQ(error_code, expected_code);
}

TEST_F(GrpcJsonClientTest, FinishAsyncCallWithInvalidTagFailsWithInvalidArgumentError) {
    intptr_t tag = 0;
    int32_t error_code = GrpcJsonClient_StartAsyncCall(
        session, "ni.grpc_json_client.TestingService", "UnaryUnaryEcho", "{}", -1, &tag);
    int32_t expected_code = static_cast<int32_t>(ErrorCode::kNone);
    ASSERT_EQ(error_code, expected_code);

    size_t size = 0;
    tag++;
    error_code = GrpcJsonClient_FinishAsyncCall(session, tag, 100, 0, &size);
    expected_code = static_cast<int32_t>(ErrorCode::kInvalidArgumentError);
    ASSERT_EQ(error_code, expected_code);
}

TEST_F(GrpcJsonClientTest, CallWithShortTimeoutFailsWithTimeoutError) {
    json request = { {"delay", 100} };
    char* service = "ni.grpc_json_client.TestingService";
    intptr_t tag = 0;
    int32_t error_code = GrpcJsonClient_StartAsyncCall(
        session, service, "UnaryUnaryEcho", request.dump().c_str(), -1, &tag);
    int32_t expected_code = static_cast<int32_t>(ErrorCode::kNone);
    ASSERT_EQ(error_code, expected_code);

    size_t size = 0;
    error_code = GrpcJsonClient_FinishAsyncCall(session, tag, 10, 0, &size);
    expected_code = static_cast<int32_t>(ErrorCode::kTimeoutError);
    ASSERT_EQ(error_code, expected_code);
}

TEST_F(GrpcJsonClientTest, FinishAsyncCallWithSmallBufferFailsWithBufferSizeOutOfRangeError) {
    intptr_t tag = 0;
    int32_t error_code = GrpcJsonClient_StartAsyncCall(
        session, "ni.grpc_json_client.TestingService", "UnaryUnaryEcho", "{}", -1, &tag);
    int32_t expected_code = static_cast<int32_t>(ErrorCode::kNone);
    ASSERT_EQ(error_code, expected_code);

    size_t size = 0;
    char* buffer = reinterpret_cast<char*>(1);
    error_code = GrpcJsonClient_FinishAsyncCall(session, tag, 100, buffer, &size);
    expected_code = static_cast<int32_t>(ErrorCode::kBufferSizeOutOfRangeError);
    ASSERT_EQ(error_code, expected_code);
}

TEST_F(GrpcJsonClientTest, GetErrorSucceeds) {
    intptr_t tag = 0;
    int32_t expected_code = GrpcJsonClient_StartAsyncCall(
        session, "UndefinedService", "UndefinedMethod", "{}", -1, &tag);

    int32_t queried_error_code = 0;
    size_t size = 0;
    int32_t get_error_error_code = GrpcJsonClient_GetError(
        session, &queried_error_code, 0, &size);
    ASSERT_EQ(get_error_error_code, 0);
    unique_ptr<char> buffer(new char[size]);
    get_error_error_code = GrpcJsonClient_GetError(
        session, &queried_error_code, buffer.get(), &size);

    ASSERT_EQ(queried_error_code, expected_code);  // returns most recent code
    expected_code = static_cast<int32_t>(ErrorCode::kNone);
    ASSERT_EQ(get_error_error_code, expected_code);
}

TEST_F(GrpcJsonClientTest, GetErrorWithSmallBufferSucceedsWithBufferSizeOutOfRangeWarning) {
    intptr_t tag = 0;
    GrpcJsonClient_StartAsyncCall(session, "UndefinedService", "UndefinedMethod", "{}", -1, &tag);

    int32_t queried_error_code = 0;
    char buffer = 0;
    size_t size = 1;
    int32_t error_code = GrpcJsonClient_GetError(session, &queried_error_code, &buffer, &size);

    int32_t expected_code = static_cast<int32_t>(ErrorCode::kBufferSizeOutOfRangeWarning);
    ASSERT_EQ(error_code, expected_code);
}

TEST_F(GrpcJsonClientTest, GetErrorStringWithoutSessionSucceeds) {
    intptr_t tag = 0;
    int32_t error_code = GrpcJsonClient_StartAsyncCall(
        session, "UndefinedService", "UndefinedMethod", "{}", -1, &tag);

    size_t size = 0;
    int32_t get_error_error_code = {
        GrpcJsonClient_GetErrorString(0, error_code, 0, &size)
    };
    int32_t expected_code = static_cast<int32_t>(ErrorCode::kNone);
    ASSERT_EQ(get_error_error_code, expected_code);
    unique_ptr<char> buffer(new char[size]);
    get_error_error_code = GrpcJsonClient_GetErrorString(0, error_code, buffer.get(), &size);
    ASSERT_EQ(get_error_error_code, expected_code);

    string expected_description = GetErrorString(static_cast<ErrorCode>(error_code));
    ASSERT_STREQ(buffer.get(), expected_description.c_str());
}

TEST_F(GrpcJsonClientNoServerReflectionTest, CallsReturnsReflectionServiceNotRunningErrorMessage) {
    intptr_t tag = 0;
    size_t size = 0;
    int32_t error_code = {
        GrpcJsonClient_BlockingCall(session, "", "", "", 100, &tag, nullptr, &size)
    };
    int32_t expected_code = static_cast<int32_t>(ErrorCode::kRemoteProcedureCallError);
    EXPECT_EQ(error_code, expected_code);

    int32_t queried_code = 0;
    string message;
    GetErrorHelper(session, &queried_code, &message);
    ASSERT_THAT(message, testing::HasSubstr("The reflection service is not running on the host."));
}

}  // namespace grpc_json_client
}  // namespace ni
