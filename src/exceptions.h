
#pragma once

#include <stdexcept>
#include <string>

#include "error_code.h"
#include "grpcpp/grpcpp.h"

namespace ni {
namespace grpc_json_client {

class JsonClientException : public std::exception {
 public:
    explicit JsonClientException(const std::string& message) : _message(message) {}
    explicit JsonClientException(const std::string& summary, const std::string& details);

    virtual ErrorCode code() const;
    const char* what() const override;
    const std::string& message() const;

 protected:
    std::string _message;
};

class RemoteProcedureCallException : public JsonClientException {
 public:
    RemoteProcedureCallException(const grpc::Status& status, const std::string& summary);
    RemoteProcedureCallException(
        const grpc::Status& status, const std::string& summary, const std::string& details);

    ErrorCode code() const override;
    const grpc::Status& status() const;

 private:
    grpc::Status _status;
};

class ReflectionServiceException : public RemoteProcedureCallException {
    using RemoteProcedureCallException::RemoteProcedureCallException;

    ErrorCode code() const override;
};

class ServiceNotFoundException : public JsonClientException {
 public:
    explicit ServiceNotFoundException(const std::string& name);

    ErrorCode code() const override;
};

class MethodNotFoundException :public JsonClientException {
 public:
    explicit MethodNotFoundException(const std::string& name);

    ErrorCode code() const override;
};

class SerializationException : public JsonClientException {
    using JsonClientException::JsonClientException;

    ErrorCode code() const override;
};

class DeserializationException : public JsonClientException {
    using JsonClientException::JsonClientException;

    ErrorCode code() const override;
};

class InvalidArgumentException : public JsonClientException {
    using JsonClientException::JsonClientException;

    ErrorCode code() const override;
};

class TimeoutException : public JsonClientException {
    using JsonClientException::JsonClientException;

    ErrorCode code() const override;
};

class BufferSizeOutOfRangeException : public JsonClientException {
    using JsonClientException::JsonClientException;

    ErrorCode code() const override;
};

}  // namespace grpc_json_client
}  // namespace ni
