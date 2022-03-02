
#pragma once

#include <stdexcept>
#include <string>

#include "error_code.h"
#include "grpcpp/grpcpp.h"

namespace ni {
namespace grpc_json_client {

class JsonClientException : public std::exception {
 public:
    explicit JsonClientException(const std::string& message) :
        _message(message) {}

    virtual ErrorCode code() const {
        return ErrorCode::kUnknownError;
    }

    const char* what() const override {
        return _message.c_str();
    }

    const std::string& message() const {
        return _message;
    }

 protected:
    std::string _message;
};

class RemoteProcedureCallException : public JsonClientException {
 public:
    RemoteProcedureCallException(const grpc::Status& grpc_status, const std::string& summary)
    : JsonClientException(summary), _status(grpc_status) {
        if (!_status.ok()) {
            _message += "\n\n";
            _message += _status.error_message();
        }
    }

    ErrorCode code() const override {
        return ErrorCode::kRemoteProcedureCallError;
    }

    const grpc::Status& status() const {
        return _status;
    }

 private:
    grpc::Status _status;
};

class ReflectionServiceException : public RemoteProcedureCallException {
    using RemoteProcedureCallException::RemoteProcedureCallException;

    ErrorCode code() const override {
        return ErrorCode::kReflectionServiceError;
    }
};

class ServiceNotFoundException : public JsonClientException {
 public:
    explicit ServiceNotFoundException(const std::string& name) :
        JsonClientException("Service descriptor not found: " + name) {}

    ErrorCode code() const override {
        return ErrorCode::kServiceNotFoundError;
    }
};

class MethodNotFoundException :public JsonClientException {
 public:
    explicit MethodNotFoundException(const std::string& name) :
        JsonClientException("Method descriptor not found: " + name) {}

    ErrorCode code() const override {
        return ErrorCode::kMethodNotFoundError;
    }
};

class SerializationException : public JsonClientException {
    using JsonClientException::JsonClientException;

    ErrorCode code() const override {
        return ErrorCode::kSerializationError;
    }
};

class DeserializationException : public JsonClientException {
    using JsonClientException::JsonClientException;

    ErrorCode code() const override {
        return ErrorCode::kDeserializationError;
    }
};

class InvalidTagException : public JsonClientException {
    using JsonClientException::JsonClientException;

    ErrorCode code() const override {
        return ErrorCode::kInvalidTagError;
    }
};

class TimeoutException : public JsonClientException {
    using JsonClientException::JsonClientException;

    ErrorCode code() const override {
        return ErrorCode::kTimeoutError;
    }
};

class BufferSizeOutOfRangeException : public JsonClientException {
    using JsonClientException::JsonClientException;

    ErrorCode code() const override {
        return ErrorCode::kBufferSizeOutOfRangeError;
    }
};

}  // namespace grpc_json_client
}  // namespace ni
