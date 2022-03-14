
#pragma once

#include <cstdint>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>

#include "grpcpp/grpcpp.h"

#include "error_code.h"
#include "unary_unary_json_client.h"

namespace ni {
namespace grpc_json_client {

class Session {
 public:
    Session(
        const std::string& target, const std::shared_ptr<grpc::ChannelCredentials>& credentials);

    int32_t ResetDescriptorDatabase();
    int32_t FillDescriptorDatabase(int32_t timeout);
    int32_t StartAsyncCall(
        const char* service, const char* method, const char* request, int32_t timeout, void** tag);
    int32_t FinishAsyncCall(void* tag, int32_t timeout, char* buffer, size_t* size);
    int32_t BlockingCall(
        const char* service,
        const char* method,
        const char* request,
        int32_t timeout,
        void** tag,
        char* response,
        size_t* size);
    int32_t Lock(int32_t timeout, uint8_t* has_lock);
    int32_t Unlock();
    int32_t GetError(int32_t* code, char* buffer, size_t* size);
    static int32_t GetErrorString(Session* session, int32_t code, char* buffer, size_t* size);
    int32_t Close();

 private:
    std::recursive_timed_mutex _lock;
    UnaryUnaryJsonClient _client;
    std::unordered_map<const void*, std::string> _responses;
    ErrorCode _error_code;
    std::string _error_message;

    // Helper function for catching exceptions.
    int32_t Evaluate(const std::function<ErrorCode(UnaryUnaryJsonClient&)>& func);
    int32_t RaiseWarning(ErrorCode warning_code, const std::string& message);
    int32_t RaiseBufferSizeOutOfRangeWarning();
};

}  // namespace grpc_json_client
}  // namespace ni
