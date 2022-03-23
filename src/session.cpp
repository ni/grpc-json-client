
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
    _client(target, credentials) {
    ClearErrorState();
}

int32_t Session::ResetDescriptorDatabase() {
    return Evaluate(
        [](UnaryUnaryJsonClient& client) {
            client.ResetDescriptorDatabase();
            return ErrorCode::kNone;
        });
}

int32_t Session::FillDescriptorDatabase(int32_t timeout) {
    system_clock::time_point deadline = DeadlineFromTimeout(timeout);
    return Evaluate(
        [&](UnaryUnaryJsonClient& client) {
            client.FillDescriptorDatabase(deadline);
            return ErrorCode::kNone;
        });
}

int32_t Session::StartAsyncCall(
    const char* service, const char* method, const char* request, int32_t timeout, void** tag
) {
    system_clock::time_point deadline = DeadlineFromTimeout(timeout);
    return StartAsyncCall(service, method, request, deadline, tag);
}

int32_t Session::FinishAsyncCall(void* tag, int32_t timeout, char* buffer, size_t* size) {
    system_clock::time_point deadline = DeadlineFromTimeout(timeout);
    return FinishAsyncCall(tag, deadline, buffer, size);
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
        // deadline will be ignored by the method
        return FinishAsyncCall(*tag, system_clock::now(), response, size);
    }
    system_clock::time_point deadline = DeadlineFromTimeout(timeout);
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

int32_t Session::Lock(int32_t timeout, uint8_t* has_lock) {
    system_clock::time_point deadline = DeadlineFromTimeout(timeout);
    return Evaluate(
        [=, &deadline](const UnaryUnaryJsonClient&) {
            if (!has_lock || !*has_lock) {
                if (_lock.try_lock_until(deadline) && has_lock) {
                    *has_lock = true;
                }
            }
            return ErrorCode::kNone;
        });
}

int32_t Session::Unlock(uint8_t* has_lock) {
    return Evaluate(
        [=](const UnaryUnaryJsonClient&) {
            if (!has_lock || *has_lock) {
                _lock.unlock();
                if (has_lock) {
                    *has_lock = false;
                }
            }
            return ErrorCode::kNone;
        });
}

int32_t Session::GetDefaultRequest(
    const char* service,
    const char* method,
    int32_t timeout,
    char* buffer,
    size_t* size) {
    system_clock::time_point deadline = DeadlineFromTimeout(timeout);
    return Evaluate(
        [=, &deadline](UnaryUnaryJsonClient& client) {
            string request = client.GetDefaultRequest(service, method, deadline);
            if (!buffer) {
                *size = request.size() + 1;  // include null char
            } else if (*size > request.size()) {  // null char
                strncpy(buffer, request.c_str(), *size);
            } else {
                string message = {
                    "The buffer size is too small to accommodate the message."
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
                *code = _error_code;
            }
            if (size) {
                if (buffer) {
                    strncpy(buffer, _error_message.c_str(), *size);
                    if (*size > _error_message.size()) {
                        ClearErrorState();
                    } else {
                        if (*size > 0) {
                            buffer[*size - 1] = NULL;  // strncpy doesn't add null char
                        }
                        return RaiseBufferSizeOutOfRangeWarning();
                    }
                } else {
                    *size = _error_message.size() + 1;  // include null char
                }
            } else {
                ClearErrorState();
            }
            return ErrorCode::kNone;
        });
}

int32_t Session::GetErrorString(Session* session, int32_t code, char* buffer, size_t* size) {
    ErrorCode error_code = static_cast<ErrorCode>(code);
    string message = ni::grpc_json_client::GetErrorString(error_code);
    message = JsonClientException::FormatErrorMessage(error_code, message, string());
    if (buffer) {
        strncpy(buffer, message.c_str(), *size);
        if (*size <= message.size()) {
            if (*size > 0) {
                buffer[*size - 1] = NULL;  // strncpy doesn't add null char
            }
            if (session) {
                session->RaiseBufferSizeOutOfRangeWarning();
            }
            return static_cast<int32_t>(ErrorCode::kBufferSizeOutOfRangeWarning);
        }
    } else {
        *size = message.size() + 1;  // include null char
    }
    return static_cast<int32_t>(ErrorCode::kNone);
}

int32_t Session::Close() {
    // placeholder for any pre-destructor cleanup operations
    return static_cast<int32_t>(ErrorCode::kNone);
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
            if (size) {
                const string& response = _responses[tag];
                if (!buffer) {
                    *size = response.size() + 1;  // include null char
                }
                else if (*size > response.size()) {  // null char
                    strncpy(buffer, response.c_str(), *size);
                    _responses.erase(tag);
                }
                else {
                    string message = {
                        "The buffer size is too small to accommodate the response."
                    };
                    throw BufferSizeOutOfRangeException(message);
                }
            }
            else {
                _responses.erase(tag);
            }
            return ErrorCode::kNone;
        });
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
        SetErrorState(ex.code(), TraceExceptions(ex));
    }
    return _error_code;
}

void Session::SetErrorState(ErrorCode code, const string& message) {
    _error_code = static_cast<int32_t>(code);
    _error_message = message;
}

void Session::ClearErrorState() {
    ErrorCode code = ErrorCode::kNone;
    string message = ni::grpc_json_client::GetErrorString(code);
    RaiseWarning(code, message);  // to format message
}

void Session::RaiseWarning(ErrorCode code, string message) {
    message = JsonClientException::FormatErrorMessage(code, message, string());
    SetErrorState(code, message);
}

ErrorCode Session::RaiseBufferSizeOutOfRangeWarning() {
    ErrorCode code = ErrorCode::kBufferSizeOutOfRangeWarning;
    string message = {
        "The buffer size is too small to accomodate the entire string. It will be truncated."
    };
    RaiseWarning(code, message);
    return code;
}

}  // namespace grpc_json_client
}  // namespace ni
