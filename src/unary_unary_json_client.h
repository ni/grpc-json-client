
#pragma once

#include <memory>
#include <string>
#include <unordered_set>

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
        const std::string& request_json);
    std::string FinishAsyncCall(void* tag, int timeout);

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
