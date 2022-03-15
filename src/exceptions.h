
#pragma once

#include <exception>
#include <string>

#include <grpcpp/grpcpp.h>

#include "error_code.h"

namespace ni {
namespace grpc_json_client {

class JsonClientException : public std::exception {
 public:
    explicit JsonClientException(const std::string& message) :
        JsonClientException(ErrorCode::kUnknownError, message, std::string())
    {}

    JsonClientException(const std::string& summary, const std::string& details) :
        JsonClientException(ErrorCode::kUnknownError, summary, details)
    {}

    static std::string FormatErrorMessage(
        ErrorCode code, const std::string& summary, const std::string& details);

    ErrorCode code() const;
    const char* what() const override;
    const std::string& message() const;

 protected:
    JsonClientException(ErrorCode code, const std::string& message) :
        JsonClientException(code, message, std::string())
    {}

    JsonClientException(ErrorCode code, const std::string& summary, const std::string& details) :
        _code(code), _message(FormatErrorMessage(code, summary, details))
    {}

 private:
    ErrorCode _code;
    std::string _message;
};

class RemoteProcedureCallException : public JsonClientException {
 public:
    RemoteProcedureCallException(const grpc::Status& status, const std::string& message) :
        RemoteProcedureCallException(
            ErrorCode::kRemoteProcedureCallError, status, message, std::string())
    {}

    RemoteProcedureCallException(
        const grpc::Status& status, const std::string& summary, const std::string& details) :
        RemoteProcedureCallException(
            ErrorCode::kRemoteProcedureCallError, status, summary, details)
    {}

    const grpc::Status& status() const;

 protected:
    RemoteProcedureCallException(
        ErrorCode code,
        const grpc::Status& status,
        const std::string& summary,
        const std::string& details) :
        JsonClientException(code, summary, AppendStatusDetails(status, details)), _status(status)
    {}

 private:
    static std::string AppendStatusDetails(const grpc::Status& status, std::string message);

    grpc::Status _status;
};

class ReflectionServiceException : public RemoteProcedureCallException {
 public:
    ReflectionServiceException(const grpc::Status& status, const std::string& message) :
        RemoteProcedureCallException(
            ErrorCode::kReflectionServiceError, status, message, std::string())
    {}

    ReflectionServiceException(
        const grpc::Status& status,
        const std::string& summary,
        const std::string& details) :
        RemoteProcedureCallException(
            ErrorCode::kReflectionServiceError, status, summary, details)
    {}
};

class ServiceNotFoundException : public JsonClientException {
 public:
    explicit ServiceNotFoundException(const std::string& name);
};

class MethodNotFoundException :public JsonClientException {
 public:
    explicit MethodNotFoundException(const std::string& name);
};

class SerializationException : public JsonClientException {
 public:
    explicit SerializationException(const std::string& message) :
        JsonClientException(ErrorCode::kSerializationError, message)
    {}

    SerializationException(const std::string& summary, const std::string& details) :
        JsonClientException(ErrorCode::kSerializationError, summary, details)
    {}
};

class DeserializationException : public JsonClientException {
 public:
    explicit DeserializationException(const std::string& message) :
        JsonClientException(ErrorCode::kDeserializationError, message)
    {}

    DeserializationException(const std::string& summary, const std::string& details) :
        JsonClientException(ErrorCode::kDeserializationError, summary, details)
    {}
};

class InvalidArgumentException : public JsonClientException {
 public:
    explicit InvalidArgumentException(const std::string& message) :
        JsonClientException(ErrorCode::kInvalidArgumentError, message)
    {}

    InvalidArgumentException(const std::string& summary, const std::string& details) :
        JsonClientException(ErrorCode::kInvalidArgumentError, summary, details)
    {}
};

class TimeoutException : public JsonClientException {
 public:
    explicit TimeoutException(const std::string& message) :
        JsonClientException(ErrorCode::kTimeoutError, message)
    {}

    TimeoutException(const std::string& summary, const std::string& details) :
        JsonClientException(ErrorCode::kTimeoutError, summary, details)
    {}
};

class BufferSizeOutOfRangeException : public JsonClientException {
 public:
    explicit BufferSizeOutOfRangeException(const std::string& message) :
        JsonClientException(ErrorCode::kBufferSizeOutOfRangeError, message)
    {}

    BufferSizeOutOfRangeException(const std::string& summary, const std::string& details) :
        JsonClientException(ErrorCode::kBufferSizeOutOfRangeError, summary, details)
    {}
};

}  // namespace grpc_json_client
}  // namespace ni
