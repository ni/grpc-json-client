
#pragma once

#include <grpcpp/grpcpp.h>

#include "testing_service.grpc.pb.h"

namespace ni {
namespace grpc_json_client {

class TestingServiceImpl final : public TestingService::Service {
    grpc::Status UnaryUnaryEcho(
        grpc::ServerContext* context,
        const UnaryUnaryEchoMessage* request,
        UnaryUnaryEchoMessage* response) override;
};

}  // namespace grpc_json_client
}  // namespace ni
