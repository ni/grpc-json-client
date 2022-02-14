#pragma once

#include <stdexcept>

#include "error_code.h"

namespace ni
{
    namespace dynclient
    {
        class DynamicClientException : public std::exception
        {
        public:
            DynamicClientException(const std::string& description) : std::exception(description.c_str()) {}

            virtual ErrorCode error_code()
            {
                return ErrorCode::UNKNOWN;
            }
        };
    
        class ReflectionServiceException : public DynamicClientException
        {
        public:
            using DynamicClientException::DynamicClientException;

            ErrorCode error_code() override
            {
                return ErrorCode::REFLECTION_ERROR;
            }
        };

        class ServiceDescriptorNotFoundException : public DynamicClientException
        {
        public:
            ServiceDescriptorNotFoundException(const std::string& name) :
                DynamicClientException("Service descriptor not found: " + name) {}

            ErrorCode error_code() override
            {
                return ErrorCode::SERVICE_NOT_FOUND;
            }
        };

        class MethodDescriptorNotFoundException :public  DynamicClientException
        {
        public:
            MethodDescriptorNotFoundException(const std::string& name) :
                DynamicClientException("Method descriptor not found: " + name) {}

            ErrorCode error_code() override
            {
                return ErrorCode::METHOD_NOT_FOUND;
            }
        };
    
        class SerializationException : public DynamicClientException
        {
            using DynamicClientException::DynamicClientException;

            ErrorCode error_code() override
            {
                return ErrorCode::SERIALIZATION_ERROR;
            }
        };
    
        class DeserializationException : public DynamicClientException
        {
            using DynamicClientException::DynamicClientException;

            ErrorCode error_code() override
            {
                return ErrorCode::DESERIALIZATION_ERROR;
            }
        };
    }
}
