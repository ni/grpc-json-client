#pragma once

#include <grpcpp/generic/generic_stub.h>

#include "json_client_base.h"

namespace ni
{
    namespace json_client
    {
        class UnaryUnaryJsonClient : public JsonClientBase
        {
        private:
            grpc::GenericStub _stub;
            std::unique_ptr<grpc::ClientContext> _context;
            grpc::CompletionQueue _completion_queue;
            const google::protobuf::MethodDescriptor* _method_type = nullptr;
            std::unique_ptr<grpc::GenericClientAsyncResponseReader> _response_reader;
            std::string _response;

        public:
            UnaryUnaryJsonClient(const std::string& target, const std::shared_ptr<grpc::ChannelCredentials>& credentials);

            void Write(const std::string& service_name, const std::string& method_name, const std::string& request_json);
            std::string Read();
        };
    }
}
