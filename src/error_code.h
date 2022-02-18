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
            kRpcError = -2,
            kServiceNotFound = -3,
            kMethodNotFound = -4,
            kSerializationError = -5,
            kDeserializationError = -6,
            kInvalidTag = -7,
            kTimeout = -8,
            kBufferSizeOutOfRange = -9
        };

        std::string GetErrorDescription(const ErrorCode& error_code);
        ErrorCode MergeErrors(ErrorCode first, ErrorCode second);
    }
}
