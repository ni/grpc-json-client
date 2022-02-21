#include "error_code.h"

#include <stdexcept>
#include <unordered_map>

using std::out_of_range;
using std::string;
using std::unordered_map;

namespace ni
{
    namespace grpc_json_client
    {
        string GetErrorDescription(const ErrorCode& error_code)
        {
            static const unordered_map<ErrorCode, char*> descriptions = {
                {ErrorCode::kBufferSizeOutOfRangeWarning, "Buffer size out of range warning"},
                {ErrorCode::kNone, "No error"},
                {ErrorCode::kUnknownError, "Unknown error code"},
                {ErrorCode::kRpcError, "Remote procedure call error"},
                {ErrorCode::kServiceNotFoundError, "Service not found error"},
                {ErrorCode::kMethodNotFoundError, "Method not found error"},
                {ErrorCode::kSerializationError, "Serialization error"},
                {ErrorCode::kDeserializationError, "Deserialization error"},
                {ErrorCode::kInvalidTagError, "Invalid tag error"},
                {ErrorCode::kTimeoutError, "Timeout error"},
                {ErrorCode::kBufferSizeOutOfRangeError, "Buffer size out of range error"}
            };
            try
            {
                return descriptions.at(error_code);
            }
            catch (const out_of_range&)
            {
                return descriptions.at(ErrorCode::kUnknownError);
            }
        }

        ErrorCode MergeErrors(ErrorCode first, ErrorCode second)
        {
            int first_value = static_cast<int>(first);
            if (first_value < 0)
            {
                return first;
            }
            int second_value = static_cast<int>(second);
            if (second_value < 0)
            {
                return second;
            }
            return first_value > 0 ? first : second;
        }
    }
}
