#pragma once

#include <grpcpp/grpcpp.h>

#include "exceptions.h"
#include "unary_unary_dynamic_client.h"

namespace ni
{
    namespace dynclient
    {
        class Session
        {
        public:
            Session(const std::string& target, const std::shared_ptr<grpc::ChannelCredentials>& credentials);

            UnaryUnaryDynamicClient& client();
            std::mutex& lock();
            DynamicClientException& last_exception();
            ErrorCode last_error_code() const;
            std::string last_error_description() const;

            void ClearLastException();

        private:
            UnaryUnaryDynamicClient _unary_unary_client;
            std::mutex _lock;
            std::unique_ptr<DynamicClientException> _last_exception;
        };
    }
}
