
#include <cstdint>
#include <memory>
#include <string>

#include "gtest/gtest.h"

#include "grpcjsonclient/grpc_json_client.h"
#include "grpc_json_client_test_base.h"
#include "helpers.h"

using std::string;
using std::unique_ptr;

namespace ni {
namespace grpc_json_client {

using GrpcJsonClientOfflineTest = GrpcJsonClientTestBase;

TEST_F(GrpcJsonClientOfflineTest, ResetDescriptorDatabaseSucceeds) {
    EXPECT_FALSE(GrpcJsonClient_ResetDescriptorDatabase(session));
}
TEST_F(GrpcJsonClientOfflineTest, FillDescriptorDatabaseFailsWithRemoteProcedureCallError) {
    int32_t error_code = GrpcJsonClient_FillDescriptorDatabase(session, -1);
    EXPECT_EQ(error_code, -2);  // ErrorCode::kRemoteProcedureCallError

    string expected_message("Failed to initiate communication with the host.");
    EXPECT_NO_FATAL_FAILURE(CheckErrorMessageHelper(session, expected_message, true));
}

TEST_F(GrpcJsonClientOfflineTest, StartAsyncCallFailsWithRemoteProcedureCallError) {
    int32_t error_code = GrpcJsonClient_StartAsyncCall(session, "", "", "", -1, 0);
    EXPECT_EQ(error_code, -2);  // ErrorCode::kRemoteProcedureCallError

    string expected_message("Failed to initiate communication with the host.");
    EXPECT_NO_FATAL_FAILURE(CheckErrorMessageHelper(session, expected_message, true));
}

TEST_F(GrpcJsonClientOfflineTest, FinishAsyncCallFailsWithInvalidTagError) {
    int32_t error_code = GrpcJsonClient_FinishAsyncCall(session, 0, -1, nullptr, nullptr);
    EXPECT_EQ(error_code, -8);  // ErrorCode::kInvalidArgumentError

    string expected_message("An active remote procedure call was not found for the specified tag.");
    EXPECT_NO_FATAL_FAILURE(CheckErrorMessageHelper(session, expected_message));
}

TEST_F(GrpcJsonClientOfflineTest, BlockingCallFailsWithRemoteProcedureCallError) {
    intptr_t tag = 0;
    int32_t error_code = {
        GrpcJsonClient_BlockingCall(session, "", "", "", -1, &tag, nullptr, nullptr)
    };
    EXPECT_EQ(error_code, -2);  // ErrorCode::kRemoteProcedureCallError

    string expected_message("Failed to initiate communication with the host.");
    EXPECT_NO_FATAL_FAILURE(CheckErrorMessageHelper(session, expected_message, true));
}

TEST_F(GrpcJsonClientOfflineTest, LockAndUnlockSessionSucceeds) {
    uint8_t has_lock = 0;
    EXPECT_FALSE(GrpcJsonClient_LockSession(session, -1, &has_lock));
    EXPECT_TRUE(has_lock);
    EXPECT_FALSE(GrpcJsonClient_UnlockSession(session));
}

TEST_F(GrpcJsonClientOfflineTest, RecursiveLockingSucceeds) {
    uint8_t has_lock = 0;
    EXPECT_FALSE(GrpcJsonClient_LockSession(session, -1, &has_lock));
    EXPECT_TRUE(has_lock);
    EXPECT_FALSE(GrpcJsonClient_LockSession(session, -1, &has_lock));
    EXPECT_TRUE(has_lock);
    EXPECT_FALSE(GrpcJsonClient_UnlockSession(session));
    EXPECT_FALSE(GrpcJsonClient_UnlockSession(session));
}

TEST_F(GrpcJsonClientOfflineTest, GetErrorWithNoErrorSucceeds) {
    int32_t error_code = 0;
    string error_message;
    EXPECT_FALSE(GetErrorHelper(session, &error_code, &error_message));
    EXPECT_FALSE(error_code);
    EXPECT_EQ(error_message, "No error");
}

TEST_F(GrpcJsonClientOfflineTest, GetErrorStringWithNoErrorSucceeds) {
    int32_t error_code = 0;
    size_t size = 0;
    EXPECT_FALSE(GrpcJsonClient_GetErrorString(session, error_code, nullptr, &size));
    unique_ptr<char> buffer(new char[size]);
    EXPECT_FALSE(GrpcJsonClient_GetErrorString(session, error_code, buffer.get(), &size));
    EXPECT_STREQ(buffer.get(), "No error");
}

TEST_F(GrpcJsonClientOfflineTest, GetErrorStringWithErrorSucceeds) {
    int32_t error_code = -1;
    size_t size = 0;
    EXPECT_FALSE(GrpcJsonClient_GetErrorString(session, error_code, nullptr, &size));
    unique_ptr<char> buffer(new char[size]);
    EXPECT_FALSE(GrpcJsonClient_GetErrorString(session, error_code, buffer.get(), &size));
    EXPECT_STREQ(buffer.get(), "Unknown error");
}

TEST_F(
    GrpcJsonClientOfflineTest,
    GetErrorStringWithOutOfRangeErrorCodeReturnsUndefinedErrorCodeMessage
) {
    int32_t error_code = INT32_MIN;
    size_t size = 0;
    EXPECT_FALSE(GrpcJsonClient_GetErrorString(session, error_code, nullptr, &size));
    unique_ptr<char> buffer(new char[size]);
    EXPECT_FALSE(GrpcJsonClient_GetErrorString(session, error_code, buffer.get(), &size));
    EXPECT_STREQ(buffer.get(), "Undefined error code");
}

TEST_F(
    GrpcJsonClientOfflineTest,
    GetErrorStringWithSmallBufferSucceedsWithBufferSizeOutOfRangeWarning
) {
    char* buffer = "";
    size_t size = 0;
    int32_t error_code = GrpcJsonClient_GetErrorString(session, 0, buffer, &size);
    EXPECT_EQ(error_code, 1);  // ErrorCode::kBufferSizeOutOfRangeWarning
}

TEST_F(GrpcJsonClientOfflineTest, GetErrorStringWithSmallBufferPopulatesErrorState) {
    char* buffer = "";
    size_t size = 0;
    int32_t error_code = GrpcJsonClient_GetErrorString(session, 0, buffer, &size);
    EXPECT_EQ(error_code, 1);  // ErrorCode::kBufferSizeOutOfRangeWarning

    string expected_message = {
        "The buffer size is too small to accomodate the entire string. It will be truncated."
    };
    EXPECT_NO_FATAL_FAILURE(CheckErrorMessageHelper(session, expected_message));
}

TEST_F(GrpcJsonClientOfflineTest, GetErrorClearsErrorState) {
    char* buffer = "";
    size_t size = 0;
    int32_t error_code = GrpcJsonClient_GetErrorString(session, 0, buffer, &size);
    EXPECT_EQ(error_code, 1);  // ErrorCode::kBufferSizeOutOfRangeWarning

    string error_message;
    EXPECT_FALSE(GetErrorHelper(session, &error_code, &error_message));
    EXPECT_FALSE(GetErrorHelper(session, &error_code, &error_message));
    EXPECT_FALSE(error_code);
    EXPECT_EQ(error_message, "No error");
}

}  // namespace grpc_json_client
}  // namespace ni
