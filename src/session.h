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

            UnaryUnaryJsonClient& client();
            std::mutex& lock();
            JsonClientException& last_exception();
            ErrorCode last_error_code() const;
            std::string last_error_description() const;

            void ClearLastException();

        private:
            UnaryUnaryJsonClient _unary_unary_client;
            std::mutex _lock;
            std::unique_ptr<JsonClientException> _last_exception;
        };
    }
}
