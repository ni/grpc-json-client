
#include "exceptions.h"

#include <cstdio>
#include <memory>

using grpc::Status;
using std::string;
using std::unique_ptr;

namespace ni {
namespace grpc_json_client {

JsonClientException::JsonClientException(const string& summary, const string& details) :
    _message(summary + "\n\n" + details) {}

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
) : JsonClientException(summary), _status(status) {
    if (!_status.ok()) {
        const char* format = "\n\ngRPC Error Code: %d\ngRPC Error Message: %s";
        int code = static_cast<int>(_status.error_code());
        string message = _status.error_message();
        int size = snprintf(nullptr, 0, format, code, message.c_str()) + 1;  // + null char
        unique_ptr<char> buffer(new char[size]);
        snprintf(buffer.get(), size, format, code, message.c_str());
        _message += buffer.get();
    }
}

RemoteProcedureCallException::RemoteProcedureCallException(
    const grpc::Status& status, const std::string& summary, const std::string& details
) : JsonClientException(summary, details) {}

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
    JsonClientException("Service not found: " + name) {}

ErrorCode ServiceNotFoundException::code() const {
    return ErrorCode::kServiceNotFoundError;
}

MethodNotFoundException::MethodNotFoundException(const string& name) :
    JsonClientException("Method not found: " + name) {}

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
