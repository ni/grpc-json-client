#pragma once

#include <grpcpp/grpcpp.h>

#include "exceptions.h"
#include "unary_unary_json_client.h"

namespace ni
{
    namespace grpc_json_client
    {
        class Session
        {
        public:
            Session(const std::string& target, const std::shared_ptr<grpc::ChannelCredentials>& credentials);

            int32_t Init();
            int32_t StartAsyncCall(const char* service, const char* method, const char* request, void** tag);
            int32_t FinishAsyncCall(void* tag, int32_t timeout, char* buffer, size_t* size);
            int32_t Close();
            static int32_t GetError(Session* session, int32_t* code, char* description, size_t* size);

            int32_t last_error_code() const;
            std::string last_error_description() const;

        private:
            std::mutex _lock;
            UnaryUnaryJsonClient _client;
            std::unique_ptr<JsonClientException> _last_exception;
            std::map<void*, std::string> _responses;

            // Helper function for locking access to the session and catching exceptions.
            int32_t Evaluate(const std::function<void(UnaryUnaryJsonClient&)>& func);
        };
    }
}
