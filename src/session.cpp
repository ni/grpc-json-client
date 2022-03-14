
#include "session.h"

#include <cstring>
#include <exception>
#include <chrono>

#include "exceptions.h"

using grpc::ChannelCredentials;
using std::chrono::milliseconds;
using std::chrono::system_clock;
using std::exception;
using std::function;
using std::shared_ptr;
using std::string;

namespace ni {
namespace grpc_json_client {

// Converts a timeout to a deadline.
system_clock::time_point DeadlineFromTimeout(int32_t timeout) {
    if (timeout < 0) {
        return system_clock::time_point::max();
    }
    return system_clock::now() + milliseconds(timeout);
}

// Builds an error message from nested exceptions.
string BuildErrorMessage(const exception& ex) {
    string message = ex.what();
    try {
        std::rethrow_if_nested(ex);
    } catch (const exception& nested) {
        message += "\n\nThe above error was directly caused by the following error:\n\n";
        message += BuildErrorMessage(nested);
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

int32_t Session::FillDescriptorDatabase(int32_t timeout) {
    return Evaluate(
        [=](UnaryUnaryJsonClient& client) {
            system_clock::time_point deadline = DeadlineFromTimeout(timeout);
            client.FillDescriptorDatabase(deadline);
            return ErrorCode::kNone;
        });
}

int32_t Session::StartAsyncCall(
    const char* service, const char* method, const char* request, int32_t timeout, void** tag
) {
    return Evaluate(
        [=](UnaryUnaryJsonClient& client) {
            system_clock::time_point deadline = DeadlineFromTimeout(timeout);
            *tag = client.StartAsyncCall(service, method, request, deadline);
            return ErrorCode::kNone;
        });
}

int32_t Session::FinishAsyncCall(void* tag, int32_t timeout, char* buffer, size_t* size) {
    return Evaluate(
        [=](UnaryUnaryJsonClient& client) {
            if (!_responses.count(tag)) {
                system_clock::time_point deadline = DeadlineFromTimeout(timeout);
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
    int32_t timeout,
    void** tag,
    char* response,
    size_t* size
) {
    if (*tag) {
        return FinishAsyncCall(*tag, timeout, response, size);
    }
    int32_t error_code = StartAsyncCall(service, method, request, timeout, tag);
    if (error_code < 0) {
        return error_code;
    }
    int32_t next_error_code = FinishAsyncCall(*tag, timeout, response, size);
    if (next_error_code < 0) {
        return next_error_code;
    }
    return error_code > 0 ? error_code : next_error_code;
}

int32_t Session::Lock(int32_t timeout, uint8_t* has_lock) {
    return Evaluate(
        [=](const UnaryUnaryJsonClient&) {
            system_clock::time_point deadline = DeadlineFromTimeout(timeout);
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
        return static_cast<int32_t>(func(_client));
    }
    catch (const JsonClientException& ex) {
        _error_code = ex.code();
        _error_message = BuildErrorMessage(ex);
    }
    catch (const exception& ex) {
        _error_code = ErrorCode::kUnknownError;
        _error_message = "An unhandled exception occurred.\n\n" + BuildErrorMessage(ex);
    }
    catch (...) {
        _error_code = ErrorCode::kUnknownError;
        _error_message = "An unhandled exception occurred.";
    }
    return static_cast<int32_t>(_error_code);
}

int32_t Session::RaiseWarning(ErrorCode warning_code, const string& message) {
    _error_code = warning_code;
    _error_message = {
        JsonClientException::FormatErrorMessage(warning_code, message, "")
    };
    return static_cast<int>(warning_code);
}

int32_t Session::RaiseBufferSizeOutOfRangeWarning() {
    string message = {
        "The buffer size is too small to accomodate the entire string. "
        "It will be truncated."
    };
    return RaiseWarning(ErrorCode::kBufferSizeOutOfRangeWarning, message);
}

}  // namespace grpc_json_client
}  // namespace ni
