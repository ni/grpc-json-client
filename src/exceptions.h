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

            virtual ErrorCode error_code()
            {
                return ErrorCode::UNKNOWN;
            }
        };
    
        class ReflectionServiceException : public JsonClientException
        {
        public:
            using JsonClientException::JsonClientException;

            ErrorCode error_code() override
            {
                return ErrorCode::REFLECTION_ERROR;
            }
        };

        class ServiceDescriptorNotFoundException : public JsonClientException
        {
        public:
            ServiceDescriptorNotFoundException(const std::string& name) :
                JsonClientException("Service descriptor not found: " + name) {}

            ErrorCode error_code() override
            {
                return ErrorCode::SERVICE_NOT_FOUND;
            }
        };

        class MethodDescriptorNotFoundException :public JsonClientException
        {
        public:
            MethodDescriptorNotFoundException(const std::string& name) :
                JsonClientException("Method descriptor not found: " + name) {}

            ErrorCode error_code() override
            {
                return ErrorCode::METHOD_NOT_FOUND;
            }
        };
    
        class SerializationException : public JsonClientException
        {
            using JsonClientException::JsonClientException;

            ErrorCode error_code() override
            {
                return ErrorCode::SERIALIZATION_ERROR;
            }
        };
    
        class DeserializationException : public JsonClientException
        {
            using JsonClientException::JsonClientException;

            ErrorCode error_code() override
            {
                return ErrorCode::DESERIALIZATION_ERROR;
            }
        };
    }
}
