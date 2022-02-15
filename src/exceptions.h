#pragma once

#include <stdexcept>

#include "error_code.h"

namespace ni
{
    namespace json_client
    {
        class JsonClientException : public std::exception
        {
        public:
            JsonClientException(const std::string& description) : std::exception(description.c_str()) {}

            virtual ErrorCode error_code() const
            {
                return ErrorCode::kUnknown;
            }
        };
    
        class ReflectionServiceException : public JsonClientException
        {
        public:
            using JsonClientException::JsonClientException;

            ErrorCode error_code() const override
            {
                return ErrorCode::kReflectionError;
            }
        };

        class ServiceDescriptorNotFoundException : public JsonClientException
        {
        public:
            ServiceDescriptorNotFoundException(const std::string& name) :
                JsonClientException("Service descriptor not found: " + name) {}

            ErrorCode error_code() const override
            {
                return ErrorCode::kServiceNotFound;
            }
        };

        class MethodDescriptorNotFoundException :public JsonClientException
        {
        public:
            MethodDescriptorNotFoundException(const std::string& name) :
                JsonClientException("Method descriptor not found: " + name) {}

            ErrorCode error_code() const override
            {
                return ErrorCode::kMethodNotFound;
            }
        };
    
        class SerializationException : public JsonClientException
        {
            using JsonClientException::JsonClientException;

            ErrorCode error_code() const override
            {
                return ErrorCode::kSerializationError;
            }
        };
    
        class DeserializationException : public JsonClientException
        {
            using JsonClientException::JsonClientException;

            ErrorCode error_code() const override
            {
                return ErrorCode::kDeserializationError;
            }
        };
    }
}
