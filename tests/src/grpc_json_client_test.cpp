
#include "grpc_json_client_test.h"

#include <cstdint>
#include <memory>
#include <string>

#include "gtest/gtest.h"
#include "nlohmann/json.hpp"

#include "grpcjsonclient/grpc_json_client.h"
#include "helpers.h"

using nlohmann::json;
using std::string;
using std::unique_ptr;

namespace ni {
namespace grpc_json_client {

void GrpcJsonClientTest::SetUpTestSuite() {
    server->EnableReflection();
    server->Start();
}

void GrpcJsonClientTest::TearDownTestSuite() {
    server->Stop();
}

TEST_F(GrpcJsonClientTest, FillDescriptorDatabaseSucceeds) {
    EXPECT_FALSE(GrpcJsonClient_FillDescriptorDatabase(session, -1));
}

TEST_F(GrpcJsonClientTest, SynchronousCallsWithTimeoutsSucceed) {
    for (string string_field : { "first", "second" }) {
        json request = { {"string_field", string_field} };
        intptr_t tag = 0;
        EXPECT_FALSE(GrpcJsonClient_StartAsyncCall(
            session, service, echo, request.dump().c_str(), 100, &tag));
        string raw_response;
        EXPECT_FALSE(FinishAsyncCallHelper(session, tag, 100, &raw_response));
        json response = json::parse(raw_response);
        EXPECT_EQ(response["string_field"], string_field);
    }
}

TEST_F(GrpcJsonClientTest, SynchronousCallsWithoutTimeoutsSucceed) {
    for (string string_field : { "first", "second" }) {
        json request = { {"delay", 100}, {"string_field", string_field} };
        intptr_t tag = 0;
        EXPECT_FALSE(GrpcJsonClient_StartAsyncCall(
            session, service, echo, request.dump().c_str(), -1, &tag));
        string raw_response;
        EXPECT_FALSE(FinishAsyncCallHelper(session, tag, -1, &raw_response));
        json response = json::parse(raw_response);
        EXPECT_EQ(response["string_field"], string_field);
    }
}

TEST_F(GrpcJsonClientTest, AsynchronousCallsWithTimeoutsSucceed) {
    intptr_t tags[] = { 0, 0 };
    string string_fields[] = { "first", "second" };
    for (size_t i = 0; i < 2; i++) {
        json request = { {"string_field", string_fields[i]} };
        EXPECT_FALSE(GrpcJsonClient_StartAsyncCall(
            session, service, echo, request.dump().c_str(), 100, &tags[i]));
    }
    for (size_t i = 0; i < 2; i++) {
        string raw_response;
        EXPECT_FALSE(FinishAsyncCallHelper(session, tags[i], 100, &raw_response));
        json response = json::parse(raw_response);
        EXPECT_EQ(response["string_field"], string_fields[i]);
    }
}

TEST_F(GrpcJsonClientTest, AsynchronousCallsWithoutTimeoutsSucceed) {
    intptr_t tags[] = { 0, 0 };
    string string_fields[] = { "first", "second" };
    for (size_t i = 0; i < 2; i++) {
        json request = { {"delay", 100}, {"string_field", string_fields[i]} };
        EXPECT_FALSE(GrpcJsonClient_StartAsyncCall(
            session, service, echo, request.dump().c_str(), -1, &tags[i]));
    }
    for (size_t i = 0; i < 2; i++) {
        string raw_response;
        EXPECT_FALSE(FinishAsyncCallHelper(session, tags[i], -1, &raw_response));
        json response = json::parse(raw_response);
        EXPECT_EQ(response["string_field"], string_fields[i]);
    }
}

TEST_F(GrpcJsonClientTest, BlockingCallSucceeds) {
    json request = { {"string_field", "BlockingCall"} };
    string raw_response;
    EXPECT_FALSE(BlockingCallHelper(
        session, service, echo, request.dump().c_str(), 100, &raw_response));
    json response = json::parse(raw_response);
    EXPECT_EQ(response["string_field"], "BlockingCall");
}

TEST_F(GrpcJsonClientTest, StartAsyncCallToUndefinedServiceFailsWithServiceNotFoundError) {
    intptr_t tag = 0;
    int32_t error_code = GrpcJsonClient_StartAsyncCall(
        session, "UndefinedService", "UndefinedMethod", "{}", -1, &tag);
    int32_t expected_code = -4;  // ErrorCode::kServiceNotFoundError
    EXPECT_EQ(error_code, expected_code);
    string expected_message("The service \"UndefinedService\" was not found.");
    EXPECT_NO_FATAL_FAILURE(CheckGetErrorHelper(session, expected_code, expected_message, true));
}

TEST_F(GrpcJsonClientTest, StartAsyncCallToUndefinedMethodFailsWithMethodNotFoundError) {
    intptr_t tag = 0;
    int32_t error_code = GrpcJsonClient_StartAsyncCall(
        session, service, "UndefinedMethod", "{}", -1, &tag);
    int32_t expected_code = -5;  // ErrorCode::kServiceNotFoundError
    EXPECT_EQ(error_code, expected_code);
    string expected_message("The method \"UndefinedMethod\" was not found.");
    EXPECT_NO_FATAL_FAILURE(CheckGetErrorHelper(session, expected_code, expected_message));
}

TEST_F(GrpcJsonClientTest, StartAsyncCallWithMalformedMessageFailsWithSerializationError) {
    intptr_t tag = 0;
    int32_t error_code = GrpcJsonClient_StartAsyncCall(
        session, service, echo, "", -1, &tag);
    int32_t expected_code = -6;  // ErrorCode::kSerializationError
    EXPECT_EQ(error_code, expected_code);
    string expected_message("Failed to create protobuf message from JSON string.");
    EXPECT_NO_FATAL_FAILURE(CheckGetErrorHelper(session, expected_code, expected_message, true));
}

TEST_F(GrpcJsonClientTest, FinishAsyncCallWithInvalidTagFailsWithInvalidArgumentError) {
    intptr_t tag = 0;
    EXPECT_FALSE(GrpcJsonClient_StartAsyncCall(session, service, echo, "{}", -1, &tag));
    int32_t error_code = FinishAsyncCallHelper(session, ++tag, -1, nullptr);
    int32_t expected_code = -8;  // ErrorCode::kInvalidArgumentError
    EXPECT_EQ(error_code, expected_code);
    string expected_message("An active remote procedure call was not found for the specified tag.");
    EXPECT_NO_FATAL_FAILURE(CheckGetErrorHelper(session, expected_code, expected_message));
}

TEST_F(GrpcJsonClientTest, CallWithShortTimeoutFailsWithTimeoutError) {
    json request = { {"delay", 100} };
    intptr_t tag = 0;
    EXPECT_FALSE(GrpcJsonClient_StartAsyncCall(
        session, service, echo, request.dump().c_str(), -1, &tag));
    int32_t error_code = FinishAsyncCallHelper(session, tag, 10, nullptr);
    int32_t expected_code = -9;  // ErrorCode::kTimeoutError
    EXPECT_EQ(error_code, expected_code);
    string expected_message("Timed out while waiting for the remote procedure call to complete.");
    EXPECT_NO_FATAL_FAILURE(CheckGetErrorHelper(session, expected_code, expected_message));
}

TEST_F(GrpcJsonClientTest, FinishAsyncCallWithSmallBufferFailsWithBufferSizeOutOfRangeError) {
    intptr_t tag = 0;
    EXPECT_FALSE(GrpcJsonClient_StartAsyncCall(session, service, echo, "{}", -1, &tag));
    char* buffer = "";
    size_t size = 0;
    int32_t error_code = GrpcJsonClient_FinishAsyncCall(session, tag, -1, buffer, &size);
    int32_t expected_code = -10;  // ErrorCode::kBufferSizeOutOfRangeError
    EXPECT_EQ(error_code, expected_code);
    string expected_message("The buffer size is too small to accommodate the response.");
    EXPECT_NO_FATAL_FAILURE(CheckGetErrorHelper(session, expected_code, expected_message));
}

TEST_F(GrpcJsonClientTest, GetDefaultRequestSucceeds) {
    string request;
    EXPECT_NO_FATAL_FAILURE(GetDefaultRequestHelper(session, service, echo, -1, &request));
    string expected = R"({"delay":0})";
    EXPECT_EQ(request, expected);
}

TEST_F(GrpcJsonClientTest, GetErrorSucceeds) {
    intptr_t tag = 0;
    int32_t expected_code = GrpcJsonClient_StartAsyncCall(
        session, "UndefinedService", "UndefinedMethod", "{}", -1, &tag);
    string expected_message("The service \"UndefinedService\" was not found.");
    EXPECT_NO_FATAL_FAILURE(CheckGetErrorHelper(session, expected_code, expected_message, true));
}

TEST_F(GrpcJsonClientTest, GetErrorWithSmallBufferSucceedsWithBufferSizeOutOfRangeWarning) {
    intptr_t tag = 0;
    EXPECT_TRUE(GrpcJsonClient_StartAsyncCall(
        session, "UndefinedService", "UndefinedMethod", "{}", -1, &tag));
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

TEST_F(GrpcJsonClientTest, GetErrorStringWithoutSessionSucceeds) {
    intptr_t tag = 0;
    int32_t error_code = GrpcJsonClient_StartAsyncCall(
        session, "UndefinedService", "UndefinedMethod", "{}", -1, &tag);
    string expected_message("Service not found");
    EXPECT_NO_FATAL_FAILURE(CheckGetErrorStringHelper(0, error_code, expected_message));
}

}  // namespace grpc_json_client
}  // namespace ni
