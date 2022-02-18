#pragma once

#include <grpcpp/grpcpp.h>

#include "exceptions.h"
#include "unary_unary_json_client.h"

namespace ni
{
    namespace json_client
    {
        class Session
        {
        public:
            Session(const std::string& target, const std::shared_ptr<grpc::ChannelCredentials>& credentials);

            int32_t Init();
            int32_t Write(const char* service, const char* method, const char* request);
            int32_t Read(int32_t timeout, char* buffer, size_t* const size);
            int32_t Close();
            static int32_t GetError(Session* session, int32_t* const code, char* const description, size_t* const size);

        private:
            std::mutex _lock;
            UnaryUnaryJsonClient _client;
            std::unique_ptr<JsonClientException> _last_exception;
            std::unique_ptr<std::string> _last_response;

            int32_t _last_error_code() const;
            std::string _last_error_description() const;

            // Helper function for locking access to the session and catching exceptions.
            int32_t Evaluate(const std::function<void(UnaryUnaryJsonClient&)>& func);
        };
    }
}
