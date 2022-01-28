#include "exports.h"
#include "dynamic_client.h"

int32_t Init(const char* target, void** const session_handle)
{
	*session_handle = new ni::DynamicClient(target);
	return 0;
}

int32_t Query(void* const session_handle, const char* service, const char* method, const char* request, void** const response_handle)
{
	ni::DynamicClient* handle = (ni::DynamicClient*)session_handle;
	string response_string = handle->Query(service, method, request);
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
