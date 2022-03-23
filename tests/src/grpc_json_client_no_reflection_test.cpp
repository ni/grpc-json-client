
#include <cstdint>
#include <string>

#include "gtest/gtest.h"

#include "grpcjsonclient/grpc_json_client.h"
#include "grpc_json_client_test_base.h"
#include "helpers.h"

using std::string;

namespace ni {
namespace grpc_json_client {

class GrpcJsonClientNoReflectionTest :
    public GrpcJsonClientTestBase, public testing::WithParamInterface<int32_t> {
 public:
    static void SetUpTestSuite() {
        server->Start();
    }
    static void TearDownTestSuite() {
        server->Stop();
    }
};

INSTANTIATE_TEST_SUITE_P(Timeouts, GrpcJsonClientNoReflectionTest, testing::Values(-1, 10));

TEST_P(
    GrpcJsonClientNoReflectionTest, FillDescriptorDatabaseFailsWithReflectionServiceNotRunningError
) {
    int32_t error_code = GrpcJsonClient_FillDescriptorDatabase(session, GetParam());
    int32_t expected_code = -2;  // ErrorCode::kRemoteProcedureCallError
    EXPECT_EQ(error_code, expected_code);
    string expected_message("The reflection service is not running on the host.");
    EXPECT_NO_FATAL_FAILURE(CheckGetErrorHelper(session, expected_code, expected_message, true));
}

TEST_P(GrpcJsonClientNoReflectionTest, StartAsyncCallFailsWithReflectionServiceNotRunningError) {
    int32_t error_code = GrpcJsonClient_StartAsyncCall(session, "", "", "", GetParam(), nullptr);
    int32_t expected_code = -2;  // ErrorCode::kRemoteProcedureCallError
    EXPECT_EQ(error_code, expected_code);
    string expected_message("The reflection service is not running on the host.");
    EXPECT_NO_FATAL_FAILURE(CheckGetErrorHelper(session, expected_code, expected_message, true));
}

TEST_P(GrpcJsonClientNoReflectionTest, BlockingCallFailsWithReflectionServiceNotRunningError) {
    int32_t error_code = BlockingCallHelper(session, "", "", "", GetParam(), nullptr);
    int32_t expected_code = -2;  // ErrorCode::kRemoteProcedureCallError
    EXPECT_EQ(error_code, expected_code);
    string expected_message("The reflection service is not running on the host.");
    EXPECT_NO_FATAL_FAILURE(CheckGetErrorHelper(session, expected_code, expected_message, true));
}

TEST_P(GrpcJsonClientNoReflectionTest, GetDefaultRequestFailsWithReflectionServiceNotRunningError) {
    int32_t error_code = GetDefaultRequestHelper(session, "", "", GetParam(), nullptr);
    int32_t expected_code = -2;  // ErrorCode::kRemoteProcedureCallError
    EXPECT_EQ(error_code, expected_code);
    string expected_message("The reflection service is not running on the host.");
    EXPECT_NO_FATAL_FAILURE(CheckGetErrorHelper(session, expected_code, expected_message, true));
}

}  // namespace grpc_json_client
}  // namespace ni
