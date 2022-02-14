#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

__declspec(dllexport) int32_t Init(const char* target, void** const session_handle);
__declspec(dllexport) int32_t Write(void* const session_handle, const char* service, const char* method, const char* request);
__declspec(dllexport) int32_t Read(void* const session_handle, char* response, size_t* const size);
__declspec(dllexport) int32_t Close(void* const session_handle);
__declspec(dllexport) int32_t GetErrorMessage(int32_t error_code, char* const message, size_t* const size);

#ifdef __cplusplus
}
#endif
