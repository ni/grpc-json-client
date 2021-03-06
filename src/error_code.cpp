
#include "error_code.h"

#include <stdexcept>
#include <string>
#include <unordered_map>

using std::out_of_range;
using std::string;
using std::unordered_map;

namespace ni {
namespace grpc_json_client {

string GetErrorString(const ErrorCode& error_code) {
    static const unordered_map<ErrorCode, char*> error_strings = {
        {ErrorCode::kBufferSizeOutOfRangeWarning, "Buffer size out of range"},
        {ErrorCode::kNone, "No error"},
        {ErrorCode::kUnknownError, "Unknown error"},
        {ErrorCode::kRemoteProcedureCallError, "Remote procedure call error"},
        {ErrorCode::kReflectionServiceError, "Reflection service error"},
        {ErrorCode::kServiceNotFoundError, "Service not found"},
        {ErrorCode::kMethodNotFoundError, "Method not found"},
        {ErrorCode::kSerializationError, "Serialization error"},
        {ErrorCode::kDeserializationError, "Deserialization error"},
        {ErrorCode::kInvalidArgumentError, "Invalid argument"},
        {ErrorCode::kTimeoutError, "Timeout"},
        {ErrorCode::kBufferSizeOutOfRangeError, "Buffer size out of range"}
    };
    try {
        return error_strings.at(error_code);
    }
    catch (const out_of_range&) {
        return "Undefined error code";
    }
}

}  // namespace grpc_json_client
}  // namespace ni
