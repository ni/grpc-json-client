
#pragma once

#include <string>
#include <memory>

#include "grpcpp/grpcpp.h"

#include "testing_service.h"

namespace ni {
namespace grpc_json_client {

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
    void Wait();
    void Stop();
};

}  // namespace grpc_json_client
}  // namespace ni
