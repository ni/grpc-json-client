
#include <cstdint>
#include <memory>
#include <string>

#include "gtest/gtest.h"

#include "grpcjsonclient/grpc_json_client.h"
#include "grpc_json_client_test_base.h"
#include "helpers.h"

using std::string;
using std::thread;
using std::unique_ptr;

namespace ni {
namespace grpc_json_client {

using GrpcJsonClientOfflineTest = GrpcJsonClientTestBase;

TEST_F(GrpcJsonClientOfflineTest, ResetDescriptorDatabaseSucceeds) {
    EXPECT_FALSE(GrpcJsonClient_ResetDescriptorDatabase(session));
}
TEST_F(GrpcJsonClientOfflineTest, FillDescriptorDatabaseFailsWithRemoteProcedureCallError) {
    int32_t error_code = GrpcJsonClient_FillDescriptorDatabase(session, -1);
    int32_t expected_code = -2;  // ErrorCode::kRemoteProcedureCallError
    EXPECT_EQ(error_code, expected_code);
    string expected_message("Failed to initiate communication with the host.");
    EXPECT_NO_FATAL_FAILURE(CheckGetErrorHelper(session, expected_code, expected_message, true));
}

TEST_F(GrpcJsonClientOfflineTest, StartAsyncCallFailsWithRemoteProcedureCallError) {
    int32_t error_code = GrpcJsonClient_StartAsyncCall(session, "", "", "", -1, 0);
    int32_t expected_code = -2;  // ErrorCode::kRemoteProcedureCallError
    EXPECT_EQ(error_code, expected_code);
    string expected_message("Failed to initiate communication with the host.");
    EXPECT_NO_FATAL_FAILURE(CheckGetErrorHelper(session, expected_code, expected_message, true));
}

TEST_F(GrpcJsonClientOfflineTest, FinishAsyncCallFailsWithInvalidTagError) {
    int32_t error_code = FinishAsyncCallHelper(session, 0, -1, nullptr);
    int32_t expected_code = -8;  // ErrorCode::kInvalidArgumentError
    EXPECT_EQ(error_code, expected_code);
    string expected_message("An active remote procedure call was not found for the specified tag.");
    EXPECT_NO_FATAL_FAILURE(CheckGetErrorHelper(session, expected_code, expected_message));
}

TEST_F(GrpcJsonClientOfflineTest, BlockingCallFailsWithRemoteProcedureCallError) {
    int32_t error_code = BlockingCallHelper(session, "", "", "", -1, nullptr);
    int32_t expected_code = -2;  // ErrorCode::kRemoteProcedureCallError
    EXPECT_EQ(error_code, expected_code);
    string expected_message("Failed to initiate communication with the host.");
    EXPECT_NO_FATAL_FAILURE(CheckGetErrorHelper(session, expected_code, expected_message, true));
}

TEST_F(GrpcJsonClientOfflineTest, LockAndUnlockSessionSucceeds) {
    uint8_t has_lock = 0;
    EXPECT_FALSE(GrpcJsonClient_LockSession(session, -1, &has_lock));
    EXPECT_TRUE(has_lock);
    EXPECT_FALSE(GrpcJsonClient_UnlockSession(session, &has_lock));
    EXPECT_FALSE(has_lock);
}

TEST_F(GrpcJsonClientOfflineTest, RecursiveLockingSucceeds) {
    uint8_t has_lock = 0;
    EXPECT_FALSE(GrpcJsonClient_LockSession(session, -1, &has_lock));
    EXPECT_TRUE(has_lock);
    EXPECT_FALSE(GrpcJsonClient_LockSession(session, -1, &has_lock));
    EXPECT_TRUE(has_lock);
    EXPECT_FALSE(GrpcJsonClient_UnlockSession(session, &has_lock));
    EXPECT_FALSE(has_lock);
    EXPECT_FALSE(GrpcJsonClient_UnlockSession(session, &has_lock));
    EXPECT_FALSE(has_lock);
}

TEST_F(GrpcJsonClientOfflineTest, GetDefaultRequestFailsWithRemoteProcedureCallError) {
    int32_t error_code = GetDefaultRequestHelper(session, service, echo, -1, nullptr);
    int32_t expected_code = -2;  // ErrorCode::kRemoteProcedureCallError
    EXPECT_EQ(error_code, expected_code);
    string expected_message("Failed to initiate communication with the host.");
    EXPECT_NO_FATAL_FAILURE(CheckGetErrorHelper(session, expected_code, expected_message, true));
}
TEST_F(GrpcJsonClientOfflineTest, GetErrorWithNoErrorSucceeds) {
    EXPECT_NO_FATAL_FAILURE(CheckGetErrorHelper(session, 0, "No error"));
}

TEST_F(GrpcJsonClientOfflineTest, GetErrorStringWithNoErrorSucceeds) {
    EXPECT_NO_FATAL_FAILURE(CheckGetErrorStringHelper(session, 0, "No error"));
}

TEST_F(GrpcJsonClientOfflineTest, GetErrorStringWithErrorSucceeds) {
    EXPECT_NO_FATAL_FAILURE(CheckGetErrorStringHelper(session, -1, "Unknown error"));
}

TEST_F(
    GrpcJsonClientOfflineTest,
    GetErrorStringWithOutOfRangeErrorCodeReturnsUndefinedErrorCodeMessage
) {
    EXPECT_NO_FATAL_FAILURE(CheckGetErrorStringHelper(session, -100, "Undefined error code"));
}

TEST_F(
    GrpcJsonClientOfflineTest,
    GetErrorStringWithSmallBufferSucceedsWithBufferSizeOutOfRangeWarning
) {
    char* buffer = "";
    size_t size = 0;
    int32_t error_code = GrpcJsonClient_GetErrorString(0, 0, buffer, &size);
    EXPECT_EQ(error_code, 1);  // ErrorCode::kBufferSizeOutOfRangeWarning
    error_code = GrpcJsonClient_GetErrorString(session, 0, buffer, &size);
    EXPECT_EQ(error_code, 1);  // ErrorCode::kBufferSizeOutOfRangeWarning
}

TEST_F(GrpcJsonClientOfflineTest, GetErrorStringWithSmallBufferPopulatesErrorState) {
    char* buffer = "";
    size_t size = 0;
    int32_t error_code = GrpcJsonClient_GetErrorString(session, 0, buffer, &size);
    int32_t expected_code = 1;  // ErrorCode::kBufferSizeOutOfRangeWarning
    EXPECT_EQ(error_code, expected_code);
    string expected_message = {
        "The buffer size is too small to accomodate the entire string. It will be truncated."
    };
    EXPECT_NO_FATAL_FAILURE(CheckGetErrorHelper(session, expected_code, expected_message));
}

TEST_F(GrpcJsonClientOfflineTest, GetErrorClearsErrorState) {
    char* buffer = "";
    size_t size = 0;
    int32_t error_code = GrpcJsonClient_GetErrorString(session, 0, buffer, &size);
    int32_t expected_code = 1;  // ErrorCode::kBufferSizeOutOfRangeWarning
    EXPECT_EQ(error_code, expected_code);
    EXPECT_FALSE(GetErrorHelper(session, nullptr, nullptr));
    EXPECT_NO_FATAL_FAILURE(CheckGetErrorHelper(session, 0, "No error"));
}

}  // namespace grpc_json_client
}  // namespace ni
