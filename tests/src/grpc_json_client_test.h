
#pragma once

#include "grpc_json_client_test_base.h"

namespace ni {
namespace grpc_json_client {

class GrpcJsonClientTest : public GrpcJsonClientTestBase {
 protected:
    static void SetUpTestSuite();
    static void TearDownTestSuite();
};

}  // namespace grpc_json_client
}  // namespace ni
