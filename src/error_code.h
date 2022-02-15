#pragma once

#include <string>

namespace ni
{
    namespace grpc_json_client
    {
        enum class ErrorCode : int
        {
            kNone = 0,
            kUnknown = -1,
            kReflectionError = -2,
            kServiceNotFound = -3,
            kMethodNotFound = -4,
            kSerializationError = -5,
            kDeserializationError = -6
        };

        std::string GetErrorDescription(const ErrorCode& error_code);
    }
}
