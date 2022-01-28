#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

__declspec(dllexport) int32_t Init(const char* target, void** const session_handle);
__declspec(dllexport) int32_t Query(void* const session_handle, const char* service, const char* method, const char* request, void** const response_handle);
__declspec(dllexport) int32_t ReadResponse(void* const response_handle, char* response, size_t* const size);
__declspec(dllexport) int32_t Close(void* const session_handle);

#ifdef __cplusplus
}
#endif
