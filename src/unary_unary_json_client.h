#pragma once

#include <grpcpp/generic/generic_stub.h>

#include "json_client_base.h"

namespace ni
{
    namespace json_client
    {
        class UnaryUnaryJsonClient : public JsonClientBase
        {
        public:
            UnaryUnaryJsonClient(const std::string& target, const std::shared_ptr<grpc::ChannelCredentials>& credentials);
            ~UnaryUnaryJsonClient();

            void Write(const std::string& service_name, const std::string& method_name, const std::string& request_json);
            std::string Read(int timeout);

            UnaryUnaryJsonClient& operator=(const UnaryUnaryJsonClient& other) = delete;
        private:
            struct AsyncCallInfo
            {
                const google::protobuf::MethodDescriptor* method_type;
                grpc::ClientContext context;
                grpc::Status status;
                grpc::ByteBuffer serialized_response;
            };

            grpc::GenericStub _stub;
            grpc::CompletionQueue _completion_queue;
        };
    }
}
