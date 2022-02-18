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
			static const array<char*, 9> error_descriptions = {
				"No error",
				"Unknown error code",
				"Remote procedure call error",
				"Service not found",
				"Method not found",
				"Serialization error",
				"Deserialization error",
				"Invalid tag",
				"Timeout"
			};
			int index = -1 * (int)error_code;
			if (index >= error_descriptions.size() || index < 0)
			{
				index = 1;  // unkown error code
			}
			return error_descriptions[index];
		}
	}
}
