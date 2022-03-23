
#include <cstdint>
#include <thread>
#include <string>

#include "gtest/gtest.h"

#include "grpcjsonclient/grpc_json_client.h"
#include "grpc_json_client_test_base.h"
#include "helpers.h"

using std::string;
using std::thread;

namespace ni {
namespace grpc_json_client {

class GrpcJsonClientOfflineTest :
    public GrpcJsonClientTestBase, public testing::WithParamInterface<int32_t> {};

INSTANTIATE_TEST_SUITE_P(Timeouts, GrpcJsonClientOfflineTest, testing::Values(-1, 0, 10));

TEST_F(GrpcJsonClientOfflineTest, ResetDescriptorDatabaseSucceeds) {
    EXPECT_FALSE(GrpcJsonClient_ResetDescriptorDatabase(session));
}

TEST_P(GrpcJsonClientOfflineTest, FillDescriptorDatabaseFailsWithRemoteProcedureCallError) {
    int32_t error_code = GrpcJsonClient_FillDescriptorDatabase(session, GetParam());
    int32_t expected_code = -2;  // ErrorCode::kRemoteProcedureCallError
    EXPECT_EQ(error_code, expected_code);
    string expected_message("Failed to initiate communication with the host.");
    EXPECT_NO_FATAL_FAILURE(CheckGetErrorHelper(session, expected_code, expected_message, true));
}

TEST_P(GrpcJsonClientOfflineTest, StartAsyncCallFailsWithRemoteProcedureCallError) {
    int32_t error_code = GrpcJsonClient_StartAsyncCall(session, "", "", "", GetParam(), 0);
    int32_t expected_code = -2;  // ErrorCode::kRemoteProcedureCallError
    EXPECT_EQ(error_code, expected_code);
    string expected_message("Failed to initiate communication with the host.");
    EXPECT_NO_FATAL_FAILURE(CheckGetErrorHelper(session, expected_code, expected_message, true));
}

TEST_P(GrpcJsonClientOfflineTest, FinishAsyncCallFailsWithInvalidTagError) {
    int32_t error_code = FinishAsyncCallHelper(session, 0, GetParam(), nullptr);
    int32_t expected_code = -8;  // ErrorCode::kInvalidArgumentError
    EXPECT_EQ(error_code, expected_code);
    string expected_message("An active remote procedure call was not found for the specified tag.");
    EXPECT_NO_FATAL_FAILURE(CheckGetErrorHelper(session, expected_code, expected_message));
}

TEST_P(GrpcJsonClientOfflineTest, BlockingCallFailsWithRemoteProcedureCallError) {
    int32_t error_code = BlockingCallHelper(session, "", "", "", GetParam(), nullptr);
    int32_t expected_code = -2;  // ErrorCode::kRemoteProcedureCallError
    EXPECT_EQ(error_code, expected_code);
    string expected_message("Failed to initiate communication with the host.");
    EXPECT_NO_FATAL_FAILURE(CheckGetErrorHelper(session, expected_code, expected_message, true));
}

TEST_P(GrpcJsonClientOfflineTest, LockAndUnlockSessionSucceeds) {
    int8_t has_lock = 0;
    EXPECT_FALSE(GrpcJsonClient_LockSession(session, GetParam(), &has_lock));
    EXPECT_TRUE(has_lock);
    EXPECT_FALSE(GrpcJsonClient_UnlockSession(session, &has_lock));
    EXPECT_FALSE(has_lock);
}

TEST_P(GrpcJsonClientOfflineTest, LockAndUnlockSessionWithNullSucceeds) {
    EXPECT_FALSE(GrpcJsonClient_LockSession(session, GetParam(), nullptr));
    EXPECT_FALSE(GrpcJsonClient_UnlockSession(session, nullptr));
}

TEST_P(GrpcJsonClientOfflineTest, RecursiveLockingSucceeds) {
    int8_t has_lock = 0;
    EXPECT_FALSE(GrpcJsonClient_LockSession(session, GetParam(), &has_lock));
    EXPECT_TRUE(has_lock);
    EXPECT_FALSE(GrpcJsonClient_LockSession(session, GetParam(), &has_lock));
    EXPECT_TRUE(has_lock);
    EXPECT_FALSE(GrpcJsonClient_UnlockSession(session, &has_lock));
    EXPECT_FALSE(has_lock);
    EXPECT_FALSE(GrpcJsonClient_UnlockSession(session, &has_lock));
    EXPECT_FALSE(has_lock);
}

TEST_F(GrpcJsonClientOfflineTest, LockSessionBlocksCompetingThread) {
    int32_t error_code = 0;
    int8_t has_lock = 0;
    auto func = [&](intptr_t session) {
        error_code = GrpcJsonClient_LockSession(session, 0, &has_lock);
    };
    EXPECT_FALSE(GrpcJsonClient_LockSession(session, 0, nullptr));
    thread competing_thread(func, session);
    competing_thread.join();
    EXPECT_FALSE(error_code);
    EXPECT_FALSE(has_lock);
    EXPECT_FALSE(GrpcJsonClient_UnlockSession(session, nullptr));
}

TEST_P(GrpcJsonClientOfflineTest, GetDefaultRequestFailsWithRemoteProcedureCallError) {
    int32_t error_code = GetDefaultRequestHelper(session, "", "", GetParam(), nullptr);
    int32_t expected_code = -2;  // ErrorCode::kRemoteProcedureCallError
    EXPECT_EQ(error_code, expected_code);
    string expected_message("Failed to initiate communication with the host.");
    EXPECT_NO_FATAL_FAILURE(CheckGetErrorHelper(session, expected_code, expected_message, true));
}

TEST_F(GrpcJsonClientOfflineTest, GetErrorWithNoErrorSucceeds) {
    EXPECT_NO_FATAL_FAILURE(CheckGetErrorHelper(session, 0, "No error"));
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

TEST_F(GrpcJsonClientOfflineTest, GetErrorWithNullSizeClearsErrorState) {
    char* buffer = "";
    size_t size = 0;
    int32_t error_code = GrpcJsonClient_GetErrorString(session, 0, buffer, &size);
    int32_t expected_code = 1;  // ErrorCode::kBufferSizeOutOfRangeWarning
    EXPECT_EQ(error_code, expected_code);
    EXPECT_FALSE(GrpcJsonClient_GetError(session, nullptr, nullptr, nullptr));
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
    // make sure also happens with session
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

}  // namespace grpc_json_client
}  // namespace ni
