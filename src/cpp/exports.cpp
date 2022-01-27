#include "exports.h"

int Init(const char* target, ni::DynamicClient** const session_handle)
{
	*session_handle = new ni::DynamicClient(target);
	return 0;
}

int Query(ni::DynamicClient* const session_handle, const char* service, const char* method, const char* request, string** const response_handle)
{
	string response_string = session_handle->Query(service, method, request);
	*response_handle = new string(response_string);
	return 0;
}

int ReadResponse(string* const response_handle, char* response, size_t* const size)
{
	if (response != nullptr)
	{
		response_handle->copy(response, *size, 0);
	}
	else if (size != nullptr)
	{
		*size = response_handle->size();
		return 0;
	}
	delete response_handle;
	return 0;
}

int Close(ni::DynamicClient* const session_handle)
{
	delete session_handle;
	return 0;
}
