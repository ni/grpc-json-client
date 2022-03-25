
#include <cstdint>
#include <string>

#include "gtest/gtest.h"
#include "nlohmann/json.hpp"

#include "grpcjsonclient/grpc_json_client.h"
#include "grpc_json_client_test_base.h"
#include "helpers.h"

using nlohmann::json;
using std::string;

namespace ni {
namespace grpc_json_client {

class GrpcJsonClientTest :
    public GrpcJsonClientTestBase, public testing::WithParamInterface<int32_t> {
 protected:
    static void SetUpTestSuite() {
        server->EnableReflection();
        server->Start();
    }

    static void TearDownTestSuite() {
        server->Stop();
    }
};

INSTANTIATE_TEST_SUITE_P(Timeouts, GrpcJsonClientTest, testing::Values(-1, 10));

TEST_P(GrpcJsonClientTest, FillDescriptorDatabaseSucceeds) {
    EXPECT_FALSE(GrpcJsonClient_FillDescriptorDatabase(session, GetParam()));
}

TEST_P(GrpcJsonClientTest, SynchronousCallsSucceed) {
    for (string string_field : { "first", "second" }) {
        json request = { {"string_field", string_field} };
        intptr_t tag = 0;
        EXPECT_FALSE(GrpcJsonClient_StartAsyncCall(
            session, service, echo, request.dump().c_str(), GetParam(), &tag));
        string raw_response;
        EXPECT_FALSE(FinishAsyncCallHelper(session, tag, GetParam(), &raw_response));
        json response = json::parse(raw_response);
        EXPECT_EQ(response["string_field"], string_field);
    }
}

TEST_P(GrpcJsonClientTest, AsynchronousCallsSucceed) {
    intptr_t tags[] = { 0, 0 };
    string string_fields[] = { "first", "second" };
    for (size_t i = 0; i < 2; i++) {
        json request = { {"string_field", string_fields[i]} };
        EXPECT_FALSE(GrpcJsonClient_StartAsyncCall(
            session, service, echo, request.dump().c_str(), GetParam(), &tags[i]));
    }
    for (size_t i = 0; i < 2; i++) {
        string raw_response;
        EXPECT_FALSE(FinishAsyncCallHelper(session, tags[i], GetParam(), &raw_response));
        json response = json::parse(raw_response);
        EXPECT_EQ(response["string_field"], string_fields[i]);
    }
}

TEST_F(GrpcJsonClientTest, AsyncCallWithShortStartTimeoutFailsWithRemoteProcedureCallError) {
    json request = { {"delay", 100} };
    intptr_t tag = 0;
    EXPECT_FALSE(GrpcJsonClient_StartAsyncCall(
        session, service, echo, request.dump().c_str(), 10, &tag));
    int32_t error_code = FinishAsyncCallHelper(session, tag, 20, nullptr);
    int32_t expected_code = -2;  // ErrorCode::kRemoteProcedureCallError
    EXPECT_EQ(error_code, expected_code);
    string expected_message = {
        "The timeout specified when the remote procedure call was started expired before the call "
        "finished or was requested to finish."
    };
    EXPECT_NO_FATAL_FAILURE(CheckGetErrorHelper(session, expected_code, expected_message, true));
}

TEST_F(GrpcJsonClientTest, AsyncCallWithShortFinishTimeoutFailsWithTimeoutError) {
    json request = { {"delay", 100} };
    intptr_t tag = 0;
    EXPECT_FALSE(GrpcJsonClient_StartAsyncCall(
        session, service, echo, request.dump().c_str(), 20, &tag));
    int32_t error_code = FinishAsyncCallHelper(session, tag, 10, nullptr);
    int32_t expected_code = -9;  // ErrorCode::kTimeoutError
    EXPECT_EQ(error_code, expected_code);
    string expected_message = {
        "The timeout expired while waiting for the remote procedure call to complete."
    };
    EXPECT_NO_FATAL_FAILURE(CheckGetErrorHelper(session, expected_code, expected_message));
}

TEST_P(GrpcJsonClientTest, BlockingCallSucceeds) {
    json request = { {"string_field", "BlockingCall"} };
    string raw_response;
    EXPECT_FALSE(BlockingCallHelper(
        session, service, echo, request.dump().c_str(), GetParam(), &raw_response));
    json response = json::parse(raw_response);
    EXPECT_EQ(response["string_field"], "BlockingCall");
}

TEST_P(GrpcJsonClientTest, StartAsyncCallToUndefinedServiceFailsWithServiceNotFoundError) {
    intptr_t tag = 0;
    int32_t error_code = GrpcJsonClient_StartAsyncCall(
        session, "UndefinedService", "UndefinedMethod", "{}", GetParam(), &tag);
    int32_t expected_code = -4;  // ErrorCode::kServiceNotFoundError
    EXPECT_EQ(error_code, expected_code);
    string expected_message("The service \"UndefinedService\" was not found.");
    EXPECT_NO_FATAL_FAILURE(CheckGetErrorHelper(session, expected_code, expected_message, true));
}

TEST_P(GrpcJsonClientTest, StartAsyncCallToUndefinedMethodFailsWithMethodNotFoundError) {
    intptr_t tag = 0;
    int32_t error_code = GrpcJsonClient_StartAsyncCall(
        session, service, "UndefinedMethod", "{}", GetParam(), &tag);
    int32_t expected_code = -5;  // ErrorCode::kServiceNotFoundError
    EXPECT_EQ(error_code, expected_code);
    string expected_message("The method \"UndefinedMethod\" was not found.");
    EXPECT_NO_FATAL_FAILURE(CheckGetErrorHelper(session, expected_code, expected_message));
}

TEST_P(GrpcJsonClientTest, FinishAsyncCallWithNullSizeArgumentSucceeds) {
    intptr_t tag = 0;
    EXPECT_FALSE(GrpcJsonClient_StartAsyncCall(session, service, echo, "{}", GetParam(), &tag));
    EXPECT_FALSE(GrpcJsonClient_FinishAsyncCall(session, tag, GetParam(), nullptr, nullptr));
    int32_t error_code = GrpcJsonClient_FinishAsyncCall(session, tag, 0, nullptr, nullptr);
    EXPECT_EQ(error_code, -8);  // ErrorCode::kInvalidArgumentError
}

TEST_P(GrpcJsonClientTest, FinishAsyncCallWithInvalidTagFailsWithInvalidArgumentError) {
    intptr_t tag = 0;
    EXPECT_FALSE(GrpcJsonClient_StartAsyncCall(session, service, echo, "{}", GetParam(), &tag));
    int32_t error_code = FinishAsyncCallHelper(session, ++tag, GetParam(), nullptr);
    int32_t expected_code = -8;  // ErrorCode::kInvalidArgumentError
    EXPECT_EQ(error_code, expected_code);
    string expected_message("An active remote procedure call was not found for the specified tag.");
    EXPECT_NO_FATAL_FAILURE(CheckGetErrorHelper(session, expected_code, expected_message));
}

TEST_P(GrpcJsonClientTest, FinishAsyncCallWithSmallBufferFailsWithBufferSizeOutOfRangeError) {
    intptr_t tag = 0;
    EXPECT_FALSE(GrpcJsonClient_StartAsyncCall(session, service, echo, "{}", GetParam(), &tag));
    char* buffer = "";
    size_t size = 0;
    int32_t error_code = GrpcJsonClient_FinishAsyncCall(session, tag, GetParam(), buffer, &size);
    int32_t expected_code = -10;  // ErrorCode::kBufferSizeOutOfRangeError
    EXPECT_EQ(error_code, expected_code);
    string expected_message("The buffer size is too small to accommodate the response.");
    EXPECT_NO_FATAL_FAILURE(CheckGetErrorHelper(session, expected_code, expected_message));
}

TEST_P(GrpcJsonClientTest, GetDefaultRequestSucceeds) {
    string request;
    EXPECT_NO_FATAL_FAILURE(GetDefaultRequestHelper(session, service, echo, GetParam(), &request));
    string expected = R"({"delay":0})";
    EXPECT_EQ(request, expected);
}

TEST_P(GrpcJsonClientTest, GetErrorSucceeds) {
    intptr_t tag = 0;
    int32_t expected_code = GrpcJsonClient_StartAsyncCall(session, "", "", "{}", GetParam(), &tag);
    string expected_message("The service \"\" was not found.");
    EXPECT_NO_FATAL_FAILURE(CheckGetErrorHelper(session, expected_code, expected_message, true));
}

TEST_P(GrpcJsonClientTest, GetErrorWithSmallBufferSucceedsWithBufferSizeOutOfRangeWarning) {
    intptr_t tag = 0;
    EXPECT_TRUE(GrpcJsonClient_StartAsyncCall(session, "", "", "{}", GetParam(), &tag));
    char buffer[15] = {};
    size_t size = 15;
    int32_t error_code = GrpcJsonClient_GetError(session, nullptr, buffer, &size);
    int32_t expected_code = 1;  // ErrorCode::kBufferSizeOutOfRangeWarning
    EXPECT_EQ(error_code, expected_code);
    EXPECT_STREQ(buffer, "Error Code: -4");
    string expected_message = {
        "The buffer size is too small to accomodate the entire string. It will be truncated."
    };
    EXPECT_NO_FATAL_FAILURE(CheckGetErrorHelper(session, expected_code, expected_message));
}

TEST_P(GrpcJsonClientTest, GetErrorStringWithoutSessionSucceeds) {
    intptr_t tag = 0;
    int32_t error_code = GrpcJsonClient_StartAsyncCall(session, "", "", "{}", GetParam(), &tag);
    string expected_message("Service not found");
    EXPECT_NO_FATAL_FAILURE(CheckGetErrorStringHelper(0, error_code, expected_message));
}

}  // namespace grpc_json_client
}  // namespace ni
