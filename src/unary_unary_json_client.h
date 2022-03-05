
#pragma once

#include <chrono>
#include <memory>
#include <string>
#include <unordered_set>

#include "google/protobuf/descriptor.h"
#include "grpcpp/grpcpp.h"
#include "grpcpp/generic/generic_stub.h"

#include "json_client_base.h"

namespace ni {
namespace grpc_json_client {

class UnaryUnaryJsonClient : public JsonClientBase {
 public:
    UnaryUnaryJsonClient(
        const std::string& target, const std::shared_ptr<grpc::ChannelCredentials>& credentials);
    ~UnaryUnaryJsonClient();

    void* StartAsyncCall(
        const std::string& service_name,
        const std::string& method_name,
        const std::string& request_json,
        const std::chrono::system_clock::time_point& deadline);
    std::string FinishAsyncCall(void* tag, const std::chrono::system_clock::time_point& deadline);

 private:
    class AsyncCallData {
     public:
        AsyncCallData();
        ~AsyncCallData();

        const google::protobuf::MethodDescriptor* method_type;
        grpc::ClientContext context;
        grpc::CompletionQueue completion_queue;
        std::unique_ptr<grpc::GenericClientAsyncResponseReader> response_reader;
    };

    grpc::GenericStub _stub;
    std::unordered_set<AsyncCallData*> _tags;
};

}  // namespace grpc_json_client
}  // namespace ni
