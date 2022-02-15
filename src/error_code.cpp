#include "error_code.h"

#include <array>

using std::array;

namespace ni
{
	namespace json_client
	{
		const char* GetErrorDescription(const ErrorCode& error_code)
		{
			static const array<char*, 7> error_descriptions = {
				"No error.",
				"Unknown error code.",
				"Reflection error.",
				"Service not found.",
				"Method not found.",
				"Serialization error.",
				"Deserialization error."
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
