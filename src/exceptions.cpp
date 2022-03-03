
#include "exceptions.h"

using std::string;
using grpc::Status;

namespace ni {
namespace grpc_json_client {

RemoteProcedureCallException::RemoteProcedureCallException(
    const Status& status, const string& summary
) : JsonClientException(summary), _status(status) {
    if (!_status.ok()) {
        _message += "\n\ngRPC Error Code: " + status.error_code();
        _message += "\ngRPC Error Message: " + _status.error_message();
    }
}

ErrorCode RemoteProcedureCallException::code() const {
    return ErrorCode::kRemoteProcedureCallError;
}

const Status& RemoteProcedureCallException::status() const {
    return _status;
}

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
