
#include "grpc_json_client_test_base.h"

#include <memory>

#include <gtest/gtest.h>

#include <grpcjsonclient/grpc_json_client.h>
#include "testing_server.h"

using std::unique_ptr;

namespace ni {
namespace grpc_json_client {

const char* GrpcJsonClientTestBase::address = "0.0.0.0:50051";
unique_ptr<TestingServer> GrpcJsonClientTestBase::server = std::make_unique<TestingServer>(address);
const char* GrpcJsonClientTestBase::service = "ni.grpc_json_client.TestingService";
const char* GrpcJsonClientTestBase::echo = "UnaryUnaryEcho";

GrpcJsonClientTestBase::GrpcJsonClientTestBase() :
    session(0)
{}

void GrpcJsonClientTestBase::SetUp() {
    ASSERT_FALSE(GrpcJsonClient_Initialize("localhost:50051", &session));
}

void GrpcJsonClientTestBase::TearDown() {
    ASSERT_FALSE(GrpcJsonClient_Close(session));
}

}  // namespace grpc_json_client
}  // namespace ni
