
#include "session.h"

#include <memory>
#include <string>

#include "error_code.h"

using grpc::ChannelCredentials;
using std::exception;
using std::function;
using std::shared_ptr;
using std::string;

namespace ni {
namespace grpc_json_client {

Session::Session(const string& target, const shared_ptr<ChannelCredentials>& credentials) :
    _client(target, credentials),
    _error_code(ErrorCode::kNone),
    _error_description(ni::grpc_json_client::GetErrorString(ErrorCode::kNone))
{}

int32_t Session::ResetDescriptorDatabase() {
    return Evaluate(
        [](UnaryUnaryJsonClient& client) {
            client.ResetDescriptorDatabase();
            return ErrorCode::kNone;
        });
}

int32_t Session::FillDescriptorDatabase() {
    return Evaluate(
        [](UnaryUnaryJsonClient& client) {
            client.FillDescriptorDatabase();
            return ErrorCode::kNone;
        });
}

int32_t Session::StartAsyncCall(
    const char* service, const char* method, const char* request, void** tag
) {
    return Evaluate(
        [=](UnaryUnaryJsonClient& client) {
            *tag = client.StartAsyncCall(service, method, request);
            return ErrorCode::kNone;
        });
}

int32_t Session::FinishAsyncCall(void* tag, int32_t timeout, char* buffer, size_t* size) {
    return Evaluate(
        [=](UnaryUnaryJsonClient& client) {
            if (!_responses.count(tag)) {
                _responses[tag] = client.FinishAsyncCall(tag, timeout);
            }
            const string& response = _responses[tag];
            if (!buffer) {
                *size = response.size() + 1;  // include null char
            } else if (*size > response.size()) {  // null char
                strncpy(buffer, response.c_str(), *size);
                _responses.erase(tag);
            } else {
                string message = {
                    "The buffer size is too small to accommodate the response."
                };
                throw BufferSizeOutOfRangeException(message);
            }
            return ErrorCode::kNone;
        });
}

int32_t Session::BlockingCall(
    const char* service,
    const char* method,
    const char* request,
    void** tag,
    int32_t timeout,
    char* response,
    size_t* size
) {
    if (*tag) {
        return FinishAsyncCall(*tag, timeout, response, size);
    }
    int32_t error_code = StartAsyncCall(service, method, request, tag);
    if (error_code >= 0) {
        int32_t next_error_code = FinishAsyncCall(*tag, timeout, response, size);
        if (next_error_code < 0) {
            error_code = next_error_code;
        }
    }
    return error_code;
}

int32_t Session::Lock() {
    return Evaluate(
        [=](const UnaryUnaryJsonClient&) {
            _lock.lock();
            return ErrorCode::kNone;
        });
}

int32_t Session::Unlock() {
    return Evaluate(
        [=](const UnaryUnaryJsonClient&) {
            _lock.unlock();
            return ErrorCode::kNone;
        });
}

int32_t Session::GetError(int32_t* code, char* buffer, size_t* size) {
    return Evaluate(
        [=](const UnaryUnaryJsonClient&) {
            *code = static_cast<int32_t>(_error_code);
            if (buffer) {
                strncpy(buffer, _error_description.c_str(), *size);
                if (*size > _error_description.size()) {
                    // clear error state
                    _error_code = ErrorCode::kNone;
                    _error_description = ni::grpc_json_client::GetErrorString(_error_code);
                } else {
                    buffer[*size - 1] = NULL;  // strncpy doesn't add null char
                    _error_code = ErrorCode::kBufferSizeOutOfRangeWarning;
                    _error_description = {
                        "The buffer size is too small to accomodate the full error message. "
                        "It will be truncated."
                    };
                    return _error_code;
                }
            } else {
                *size = _error_description.size() + 1;  // include null char
            }
            return ErrorCode::kNone;
        });
}

int32_t Session::GetErrorString(Session* session, int32_t code, char* buffer, size_t* size) {
    string description = {
        ni::grpc_json_client::GetErrorString(static_cast<ErrorCode>(code))
    };
    if (buffer) {
        strncpy(buffer, description.c_str(), *size);
        if (*size <= description.size()) {
            buffer[*size - 1] = NULL;  // strncpy doesn't add null char
            return static_cast<int32_t>(ErrorCode::kBufferSizeOutOfRangeWarning);
        }
    } else {
        *size = description.size() + 1;  // include null char
    }
    return static_cast<int32_t>(ErrorCode::kNone);
}

int32_t Session::Close() {
    // placeholder for any pre-destructor cleanup operations
    return static_cast<int32_t>(ErrorCode::kNone);
}

int32_t Session::Evaluate(const function<ErrorCode(UnaryUnaryJsonClient&)>& func) {
    try {
        return static_cast<int32_t>(func(_client));
    }
    catch (const JsonClientException& ex) {
        _error_code = ex.code();
        _error_description = ex.message();
    }
    catch (const exception& ex) {
        _error_code = ErrorCode::kUnknownError;
        _error_description = string("An unhandled exception occurred.\n\n") + ex.what();
    }
    catch (...) {
        _error_code = ErrorCode::kUnknownError;
        _error_description = "An unhandled exception occurred.";
    }
    return static_cast<int32_t>(_error_code);
}

}  // namespace grpc_json_client
}  // namespace ni
