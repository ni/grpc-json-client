#include "unary_unary_dynamic_client.h"
#include "error_code.h"
#include "exceptions.h"
#include "exports.h"

using ni::ErrorCode;
using ni::UnaryUnaryDynamicClient;
using std::string;

int32_t Init(const char* target, void** const session_handle)
{
	*session_handle = new UnaryUnaryDynamicClient(target);
	return 0;
}

int32_t Write(void* const session_handle, const char* service, const char* method, const char* request)
{
	UnaryUnaryDynamicClient* handle = (UnaryUnaryDynamicClient*)session_handle;
	try
	{
		handle->Write(service, method, request);
	}
	catch (ni::ServiceDescriptorNotFoundException&)
	{
		return ErrorCode::SERVICE_NOT_FOUND;
	}
	catch (ni::MethodDescriptorNotFoundException&)
	{
		return ErrorCode::METHOD_NOT_FOUND;
	}
	catch (...)
	{
		return ErrorCode::UNKNOWN;
	}
	return 0;
}

int32_t Read(void* const session_handle, char* buffer, size_t* const size)
{
	UnaryUnaryDynamicClient* handle = (UnaryUnaryDynamicClient*)session_handle;
	const string* response = handle->Read();
	if (buffer != nullptr)
	{
		response->copy(buffer, *size, 0);
	}
	else if (size != nullptr)
	{
		*size = response->size();
	}
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
