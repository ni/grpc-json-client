
#include "exceptions.h"

#include <cstdio>
#include <memory>

using grpc::Status;
using std::string;
using std::unique_ptr;

namespace ni {
namespace grpc_json_client {

template <typename... Arguments>
std::string FormatString(const std::string& format, const Arguments&... args) {
    int size = snprintf(nullptr, 0, format.c_str(), args...) + 1;  // + null char
    std::unique_ptr<char> buffer(new char[size]);
    snprintf(buffer.get(), size, format.c_str(), args...);
    return buffer.get();
}

JsonClientException::JsonClientException(const std::string& message) :
    JsonClientException(message, string()) {}

JsonClientException::JsonClientException(const string& summary, const string& details) {
    const char* format = "Error Code: %d\nError Message:\n%s";
    _message = FormatString(format, static_cast<int>(code()), summary);
    if (!details.empty()) {
        _message += "\n\n" + details;
    }
}

ErrorCode JsonClientException::code() const {
    return ErrorCode::kUnknownError;
}

const char* JsonClientException::what() const {
    return _message.c_str();
}

const string& JsonClientException::message() const {
    return _message;
}

RemoteProcedureCallException::RemoteProcedureCallException(
    const Status& status, const string& summary
) : RemoteProcedureCallException(status, summary, string()) {}

RemoteProcedureCallException::RemoteProcedureCallException(
    const grpc::Status& status, const std::string& summary, const std::string& details
) : JsonClientException(summary, details), _status(status) {
    if (!_status.ok()) {
        const char* format = "\n\ngRPC Error Code: %d\ngRPC Error Message:\n%s";
        _message += {
            FormatString(format, static_cast<int>(_status.error_code()), _status.error_message())
        };
    }
}

ErrorCode RemoteProcedureCallException::code() const {
    return ErrorCode::kRemoteProcedureCallError;
}

const Status& RemoteProcedureCallException::status() const {
    return _status;
}

ErrorCode ReflectionServiceException::code() const {
    return ErrorCode::kReflectionServiceError;
}

ServiceNotFoundException::ServiceNotFoundException(const string& name) :
    JsonClientException("The service \"" + name + "\" was not found.") {}

ErrorCode ServiceNotFoundException::code() const {
    return ErrorCode::kServiceNotFoundError;
}

MethodNotFoundException::MethodNotFoundException(const string& name) :
    JsonClientException("The method \"" + name + "\" was not found.") {}

ErrorCode MethodNotFoundException::code() const {
    return ErrorCode::kMethodNotFoundError;
}

ErrorCode SerializationException::code() const {
    return ErrorCode::kSerializationError;
}

ErrorCode DeserializationException::code() const {
    return ErrorCode::kDeserializationError;
}

ErrorCode InvalidArgumentException::code() const {
    return ErrorCode::kInvalidArgumentError;
}

ErrorCode TimeoutException::code() const {
    return ErrorCode::kTimeoutError;
}

ErrorCode BufferSizeOutOfRangeException::code() const {
    return ErrorCode::kBufferSizeOutOfRangeError;
}

}  // namespace grpc_json_client
}  // namespace ni
