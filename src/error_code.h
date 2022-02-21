#pragma once

#include <string>

namespace ni
{
    namespace grpc_json_client
    {
        enum class ErrorCode : int
        {
            kBufferSizeOutOfRangeWarning = 1,
            kNone = 0,
            kUnknownError = -1,
            kRemoteProcedureCallError = -2,
            kServiceNotFoundError = -3,
            kMethodNotFoundError = -4,
            kSerializationError = -5,
            kDeserializationError = -6,
            kInvalidTagError = -7,
            kTimeoutError = -8,
            kBufferSizeOutOfRangeError = -9
        };

        std::string GetErrorDescription(const ErrorCode& error_code);
    }
}
