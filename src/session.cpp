
#include "session.h"

#include <chrono>
#include <cstdint>
#include <cstring>
#include <exception>
#include <functional>
#include <memory>
#include <string>

#include "grpcpp/grpcpp.h"

#include "error_code.h"
#include "exceptions.h"
#include "unary_unary_json_client.h"

using grpc::ChannelCredentials;
using std::chrono::system_clock;
using std::exception;
using std::function;
using std::shared_ptr;
using std::string;

namespace ni {
namespace grpc_json_client {

// Builds an error message from nested exceptions.
string TraceExceptions(const exception& ex) {
    string message = ex.what();
    try {
        std::rethrow_if_nested(ex);
    } catch (const exception& nested) {
        message += "\n\nThe above error was directly caused by the following error:\n\n";
        message += TraceExceptions(nested);
    }
    return message;
}

Session::Session(const string& target, const shared_ptr<ChannelCredentials>& credentials) :
    _client(target, credentials),
    _error_code(ErrorCode::kNone),
    _error_message(ni::grpc_json_client::GetErrorString(ErrorCode::kNone))
{}

int32_t Session::ResetDescriptorDatabase() {
    return Evaluate(
        [](UnaryUnaryJsonClient& client) {
            client.ResetDescriptorDatabase();
            return ErrorCode::kNone;
        });
}

int32_t Session::FillDescriptorDatabase(const system_clock::time_point& deadline) {
    return Evaluate(
        [&](UnaryUnaryJsonClient& client) {
            client.FillDescriptorDatabase(deadline);
            return ErrorCode::kNone;
        });
}

int32_t Session::StartAsyncCall(
    const char* service,
    const char* method,
    const char* request,
    const system_clock::time_point& deadline,
    void** tag
) {
    return Evaluate(
        [=, &deadline](UnaryUnaryJsonClient& client) {
            *tag = client.StartAsyncCall(service, method, request, deadline);
            return ErrorCode::kNone;
        });
}

int32_t Session::FinishAsyncCall(
    void* tag, const system_clock::time_point& deadline, char* buffer, size_t* size
) {
    return Evaluate(
        [=, &deadline](UnaryUnaryJsonClient& client) {
            if (!_responses.count(tag)) {
                _responses[tag] = client.FinishAsyncCall(tag, deadline);
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
    const system_clock::time_point& deadline,
    void** tag,
    char* response,
    size_t* size
) {
    if (*tag) {
        return FinishAsyncCall(*tag, deadline, response, size);
    }
    int32_t error_code = StartAsyncCall(service, method, request, deadline, tag);
    if (error_code < 0) {
        return error_code;
    }
    int32_t next_error_code = FinishAsyncCall(*tag, deadline, response, size);
    if (next_error_code < 0) {
        return next_error_code;
    }
    return error_code > 0 ? error_code : next_error_code;
}

int32_t Session::Lock(const system_clock::time_point& deadline, uint8_t* has_lock) {
    return Evaluate(
        [=, &deadline](const UnaryUnaryJsonClient&) {
            *has_lock = _lock.try_lock_until(deadline);
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

int32_t Session::GetDefaultRequest(
    const char* service,
    const char* method,
    const system_clock::time_point& deadline,
    char* buffer,
    size_t* size) {
    return Evaluate(
        [=, &deadline](UnaryUnaryJsonClient& client) {
            string request = client.GetDefaultRequest(service, method, deadline);
            if (!buffer) {
                *size = request.size() + 1;  // include null char
            }
            else if (*size > request.size()) {  // null char
                strncpy(buffer, request.c_str(), *size);
            }
            else {
                string message = {
                    "The buffer size is too small to accommodate the response."
                };
                throw BufferSizeOutOfRangeException(message);
            }
            return ErrorCode::kNone;
        });
}

int32_t Session::GetError(int32_t* code, char* buffer, size_t* size) {
    return Evaluate(
        [=](const UnaryUnaryJsonClient&) {
            if (code) {
                *code = static_cast<int32_t>(_error_code);
            }
            if (buffer) {
                strncpy(buffer, _error_message.c_str(), *size);
                if (*size > _error_message.size()) {
                    // clear error state
                    _error_code = ErrorCode::kNone;
                    _error_message = ni::grpc_json_client::GetErrorString(_error_code);
                } else {
                    if (*size > 0) {
                        buffer[*size - 1] = NULL;  // strncpy doesn't add null char
                    }
                    return static_cast<ErrorCode>(RaiseBufferSizeOutOfRangeWarning());
                }
            } else {
                *size = _error_message.size() + 1;  // include null char
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
            if (*size > 0) {
                buffer[*size - 1] = NULL;  // strncpy doesn't add null char
            }
            if (session) {
                return session->RaiseBufferSizeOutOfRangeWarning();
            }
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
        try {
            return static_cast<int32_t>(func(_client));
        }
        catch (const JsonClientException& ex) {
            throw;
        }
        catch (const exception& ex) {
            throw JsonClientException("An unhandled exception occurred.", ex.what());
        }
        catch (...) {
            throw JsonClientException("An unhandled exception occurred.");
        }
    }
    catch (const JsonClientException& ex) {
        _error_code = ex.code();
        _error_message = TraceExceptions(ex);
    }
    return static_cast<int32_t>(_error_code);
}

int32_t Session::RaiseWarning(ErrorCode warning_code, const string& message) {
    _error_code = warning_code;
    _error_message = JsonClientException::FormatErrorMessage(warning_code, message, string());
    return static_cast<int>(warning_code);
}

int32_t Session::RaiseBufferSizeOutOfRangeWarning() {
    string message = {
        "The buffer size is too small to accomodate the entire string. It will be truncated."
    };
    return RaiseWarning(ErrorCode::kBufferSizeOutOfRangeWarning, message);
}

}  // namespace grpc_json_client
}  // namespace ni
