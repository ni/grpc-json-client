#pragma once

#include <stdexcept>
#include <string>

#include "error_code.h"

namespace ni {
namespace grpc_json_client {

class JsonClientException : public std::exception {
 public:
    explicit JsonClientException(const std::string& description) :
        std::exception(description.c_str()) {}

    virtual ErrorCode error_code() const {
        return ErrorCode::kUnknownError;
    }
};

class RemoteProcedureCallException : public JsonClientException {
 public:
    using JsonClientException::JsonClientException;

    ErrorCode error_code() const override {
        return ErrorCode::kRemoteProcedureCallError;
    }
};

class ServiceDescriptorNotFoundException : public JsonClientException {
 public:
    explicit ServiceDescriptorNotFoundException(const std::string& name) :
        JsonClientException("Service descriptor not found: " + name) {}

    ErrorCode error_code() const override {
        return ErrorCode::kServiceNotFoundError;
    }
};

class MethodDescriptorNotFoundException :public JsonClientException {
 public:
    explicit MethodDescriptorNotFoundException(const std::string& name) :
        JsonClientException("Method descriptor not found: " + name) {}

    ErrorCode error_code() const override {
        return ErrorCode::kMethodNotFoundError;
    }
};

class SerializationException : public JsonClientException {
    using JsonClientException::JsonClientException;

    ErrorCode error_code() const override {
        return ErrorCode::kSerializationError;
    }
};

class DeserializationException : public JsonClientException {
    using JsonClientException::JsonClientException;

    ErrorCode error_code() const override {
        return ErrorCode::kDeserializationError;
    }
};

class InvalidTagException : public JsonClientException {
    using JsonClientException::JsonClientException;

    ErrorCode error_code() const override {
        return ErrorCode::kInvalidTagError;
    }
};

class TimeoutException : public JsonClientException {
    using JsonClientException::JsonClientException;

    ErrorCode error_code() const override {
        return ErrorCode::kTimeoutError;
    }
};

class BufferSizeOutOfRangeException : public JsonClientException {
    using JsonClientException::JsonClientException;

    ErrorCode error_code() const override {
        return ErrorCode::kBufferSizeOutOfRangeError;
    }
};

}  // namespace grpc_json_client
}  // namespace ni
