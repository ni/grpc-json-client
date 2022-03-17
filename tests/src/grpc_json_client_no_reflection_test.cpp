
#include "grpc_json_client_no_reflection_test.h"

#include <cstdint>
#include <memory>
#include <string>

#include "gtest/gtest.h"

#include "grpcjsonclient/grpc_json_client.h"
#include "helpers.h"

using std::string;
using std::unique_ptr;

namespace ni {
namespace grpc_json_client {

void GrpcJsonClientNoReflectionTest::SetUpTestSuite() {
    server->Start();
}

void GrpcJsonClientNoReflectionTest::TearDownTestSuite() {
    server->Stop();
}

TEST_F(GrpcJsonClientNoReflectionTest, CallsReturnsReflectionServiceNotRunningErrorMessage) {
    intptr_t tag = 0;
    size_t size = 0;
    int32_t error_code = BlockingCallHelper(session, "", "", "", 100, nullptr);
    EXPECT_EQ(error_code, -2);  // ErrorCode::kRemoteProcedureCallError

    string expected_message("The reflection service is not running on the host.");
    EXPECT_NO_FATAL_FAILURE(CheckErrorMessageHelper(session, expected_message, true));
}

}  // namespace grpc_json_client
}  // namespace ni
