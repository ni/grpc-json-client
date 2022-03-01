
#pragma once

#include <memory>
#include <string>
#include <unordered_map>

#include "grpcpp/grpcpp.h"

#include "exceptions.h"
#include "unary_unary_json_client.h"

namespace ni {
namespace grpc_json_client {

class Session {
 public:
    Session(
        const std::string& target, const std::shared_ptr<grpc::ChannelCredentials>& credentials);

    int32_t QueryReflectionService();
    int32_t StartAsyncCall(
        const char* service, const char* method, const char* request, void** tag);
    int32_t FinishAsyncCall(void* tag, int32_t timeout, char* buffer, size_t* size);
    int32_t BlockingCall(
        const char* service,
        const char* method,
        const char* request,
        void** tag,
        int32_t timeout,
        char* response,
        size_t* size);
    int32_t Lock();
    int32_t Unlock();
    int32_t GetError(int32_t* code, char* buffer, size_t* size);
    static int32_t GetErrorString(Session* session, int32_t code, char* buffer, size_t* size);
    int32_t Close();

 private:
    std::recursive_mutex _lock;
    UnaryUnaryJsonClient _client;
    std::unordered_map<void*, std::string> _responses;
    ErrorCode _error_code;
    std::string _error_description;

    // Helper function for locking access to the session and catching exceptions.
    int32_t Evaluate(const std::function<ErrorCode(UnaryUnaryJsonClient&)>& func);
};

}  // namespace grpc_json_client
}  // namespace ni
