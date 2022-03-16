
#include <cstdint>

#include <gtest/gtest.h>

#include <grpcjsonclient/grpc_json_client.h>
#include "grpc_json_client_test.h"
#include "helpers.h"

using std::string;

namespace ni {
namespace grpc_json_client {

using GrpcJsonClientSerializerTest = GrpcJsonClientTest;

TEST_F(GrpcJsonClientSerializerTest, ValidJsonObjectSucceeds) {
    const char* valid_json = R"({"delay":0})";
    intptr_t tag = 0;
    size_t size = 0;
    EXPECT_FALSE(GrpcJsonClient_BlockingCall(
        session, service, echo, valid_json, -1, &tag, nullptr, &size));
}

TEST_F(GrpcJsonClientSerializerTest, EmptyJsonObjectSucceeds) {
    intptr_t tag = 0;
    size_t size = 0;
    EXPECT_FALSE(GrpcJsonClient_BlockingCall(
        session, service, echo, "{}", -1, &tag, nullptr, &size));
}

TEST_F(GrpcJsonClientSerializerTest, EmptyStringFails) {
    intptr_t tag = 0;
    int32_t error_code = GrpcJsonClient_BlockingCall(
        session, service, echo, "", -1, &tag, nullptr, 0);
    EXPECT_EQ(error_code, -6);  // ErrorCode::kSerializationError

    string expected_message = "Failed to create protobuf message from JSON string.";
    EXPECT_NO_FATAL_FAILURE(CheckErrorMessageHelper(session, expected_message, true));
}

TEST_F(GrpcJsonClientSerializerTest, MalformedStringFails) {
    const char* missing_last_curly_brace = R"({"delay":0)";
    intptr_t tag = 0;
    int32_t error_code = GrpcJsonClient_BlockingCall(
        session, service, echo, "", -1, &tag, nullptr, 0);
    EXPECT_EQ(error_code, -6);  // ErrorCode::kSerializationError

    string expected_message = "Failed to create protobuf message from JSON string.";
    EXPECT_NO_FATAL_FAILURE(CheckErrorMessageHelper(session, expected_message, true));
}

TEST_F(GrpcJsonClientSerializerTest, ObjectWithInvalidPropertyFails) {
    const char* has_extra_field = R"({"delay":0,"invalid_field":"invalid_value"})";
    intptr_t tag = 0;
    int32_t error_code = GrpcJsonClient_BlockingCall(
        session, service, echo, has_extra_field, -1, &tag, nullptr, 0);
    EXPECT_EQ(error_code, -6);  // ErrorCode::kSerializationError

    string expected_message = "Failed to create protobuf message from JSON string.";
    EXPECT_NO_FATAL_FAILURE(CheckErrorMessageHelper(session, expected_message, true));
}

}  // namespace grpc_json_client
}  // namespace ni
