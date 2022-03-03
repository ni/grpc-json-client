
#pragma once

#include <string>

namespace ni {
namespace grpc_json_client {

enum class ErrorCode : int {
    kBufferSizeOutOfRangeWarning = 1,
    kNone = 0,
    kUnknownError = -1,
    kRemoteProcedureCallError = -2,
    kReflectionServiceError = -3,
    kServiceNotFoundError = -4,
    kMethodNotFoundError = -5,
    kSerializationError = -6,
    kDeserializationError = -7,
    kInvalidArgumentError = -8,
    kTimeoutError = -9,
    kBufferSizeOutOfRangeError = -10
};

std::string GetErrorString(const ErrorCode& error_code);

}  // namespace grpc_json_client
}  // namespace ni
