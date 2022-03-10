
#pragma once

#include <string>
#include <memory>

#include "grpcpp/grpcpp.h"

#include "testing_service.grpc.pb.h"

namespace ni {
namespace grpc_json_client {

class TestingServiceImpl final : public TestingService::Service {
    grpc::Status UnaryUnaryEcho(
        grpc::ServerContext* context,
        const UnaryUnaryEchoMessage* request,
        UnaryUnaryEchoMessage* response) override;
};

class TestingServer {
 private:
    std::string _address;
    TestingServiceImpl _service;
    std::unique_ptr<grpc::Server> _server;
    bool _reflection_enabled;

 public:
    explicit TestingServer(const std::string& address);
    void EnableReflection();
    void Start();
    void Stop();
};

}  // namespace grpc_json_client
}  // namespace ni
