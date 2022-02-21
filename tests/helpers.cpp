#include "helpers.h"

#include "grpc_json_client.h"

using std::string;

int32_t MergeErrors(int32_t first, int32_t second)
{
	if (first < 0)
	{
		return first;
	}
	if (second < 0)
	{
		return second;
	}
	return first > 0 ? first : second;
}

int32_t BlockingCall(void* session_handle, const string& service, const string& method, const string& request, string& response)
{
	int32_t merged_error_code = 0;
	void* tag = nullptr;
	int32_t error_code = GrpcJsonClient_StartAsyncCall(
		session_handle, service.c_str(), method.c_str(), request.c_str(), &tag
	);
	if (error_code < 0)
	{
		return error_code;
	}
	merged_error_code = MergeErrors(merged_error_code, error_code);
	size_t size;
	error_code = GrpcJsonClient_FinishAsyncCall(session_handle, tag, -1, nullptr, &size);
	if (error_code < 0)
	{
		return error_code;
	}
	merged_error_code = MergeErrors(merged_error_code, error_code);
	char* buffer = new char[size];
	error_code = GrpcJsonClient_FinishAsyncCall(session_handle, tag, -1, buffer, &size);
	merged_error_code = MergeErrors(merged_error_code, error_code);
	response = string(buffer);
	delete[] buffer;
	return merged_error_code;
}
