
#include <cstdint>

#include "gtest/gtest.h"

#include "grpcjsonclient/grpc_json_client.h"
#include "grpc_json_client_test.h"
#include "helpers.h"

using std::string;

namespace ni {
namespace grpc_json_client {

using GrpcJsonClientSerializerTest = GrpcJsonClientTest;

TEST_F(GrpcJsonClientSerializerTest, ValidJsonObjectSucceeds) {
    const char* valid_json = R"({"delay":0})";
    EXPECT_FALSE(BlockingCallHelper(session, service, echo, valid_json, -1, nullptr));
}

TEST_F(GrpcJsonClientSerializerTest, EmptyJsonObjectSucceeds) {
    EXPECT_FALSE(BlockingCallHelper(session, service, echo, "{}", -1, nullptr));
}

TEST_F(GrpcJsonClientSerializerTest, EmptyStringFails) {
    int32_t error_code = BlockingCallHelper(session, service, echo, "", -1, nullptr);
    int32_t expected_code = -6;  // ErrorCode::kSerializationError
    EXPECT_EQ(error_code, expected_code);
    string expected_message = "Failed to create protobuf message from JSON string.";
    EXPECT_NO_FATAL_FAILURE(CheckGetErrorHelper(session, expected_code, expected_message, true));
}

TEST_F(GrpcJsonClientSerializerTest, MalformedStringFails) {
    const char* missing_last_curly_brace = R"({"delay":0)";
    int32_t error_code = BlockingCallHelper(session, service, echo, "", -1, nullptr);
    int32_t expected_code = -6;  // ErrorCode::kSerializationError
    EXPECT_EQ(error_code, expected_code);
    string expected_message = "Failed to create protobuf message from JSON string.";
    EXPECT_NO_FATAL_FAILURE(CheckGetErrorHelper(session, expected_code, expected_message, true));
}

TEST_F(GrpcJsonClientSerializerTest, ObjectWithInvalidPropertyFails) {
    const char* has_extra_field = R"({"delay":0,"invalid_field":"invalid_value"})";
    int32_t error_code = BlockingCallHelper(session, service, echo, has_extra_field, -1, nullptr);
    int32_t expected_code = -6;  // ErrorCode::kSerializationError
    EXPECT_EQ(error_code, expected_code);
    string expected_message = "Failed to create protobuf message from JSON string.";
    EXPECT_NO_FATAL_FAILURE(CheckGetErrorHelper(session, expected_code, expected_message, true));
}

}  // namespace grpc_json_client
}  // namespace ni
