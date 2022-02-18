#include "error_code.h"

#include <array>

using std::array;
using std::string;

namespace ni
{
    namespace grpc_json_client
    {
        string GetErrorDescription(const ErrorCode& error_code)
        {
            static const array<char*, 10> error_descriptions = {
                "No error",
                "Unknown error code",
                "Remote procedure call error",
                "Service not found",
                "Method not found",
                "Serialization error",
                "Deserialization error",
                "Invalid tag",
                "Timeout",
                "Buffer size out of range"
            };
            int index = -1 * (int)error_code;
            if (index >= error_descriptions.size() || index < 0)
            {
                index = 1;  // unkown error code
            }
            return error_descriptions[index];
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
