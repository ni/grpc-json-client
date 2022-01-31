#include "dynamic_client.h"#
#include "error_code.h"
#include "exceptions.h"
#include "exports.h"

using namespace std;
using namespace ni;

int32_t Init(const char* target, void** const session_handle)
{
	*session_handle = new DynamicClient(target);
	return 0;
}

int32_t Query(void* const session_handle, const char* service, const char* method, const char* request, void** const response_handle)
{
	DynamicClient* handle = (DynamicClient*)session_handle;
	string response_string;
	try
	{
		response_string = handle->Query(service, method, request);
	}
	catch (ServiceNotFoundException&)
	{
		return ErrorCode::SERVICE_NOT_FOUND;
	}
	catch (MethodNotFoundException&)
	{
		return ErrorCode::METHOD_NOT_FOUND;
	}
	catch (...)
	{
		return ErrorCode::UNKNOWN;
	}
	*response_handle = new string(response_string);
	return 0;
}

int32_t ReadResponse(void* const response_handle, char* response, size_t* const size)
{
	string* handle = (string*)response_handle;
	if (response != nullptr)
	{
		handle->copy(response, *size, 0);
	}
	else if (size != nullptr)
	{
		*size = handle->size();
		return 0;
	}
	delete response_handle;
	return 0;
}

int32_t Close(void* const session_handle)
{
	delete session_handle;
	return 0;
}

int32_t GetErrorMessage(int32_t error_code, char* const message, size_t* const size)
{
	string error_message;
	switch (error_code)
	{
	case ErrorCode::NONE:
		break;
	case ErrorCode::SERVICE_NOT_FOUND:
		error_message = "Service not found";
		break;
	case ErrorCode::METHOD_NOT_FOUND:
		error_message = "Method not found";
		break;
	default:
		error_message = "Error code is undefined";
	}

	if (message != nullptr)
	{
		error_message.copy(message, *size, 0);
	}
	else if (size != nullptr)
	{
		*size = error_message.size();
	}
	return 0;
}
