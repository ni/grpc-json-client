#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

// Initialize a session with an insecure channel.
__declspec(dllexport) int32_t InitInsecure(const char* target, void** const session_handle);

// Send a message to the host.
__declspec(dllexport) int32_t Write(void* const session_handle, const char* service, const char* method, const char* request);

// Read a response from the host.
__declspec(dllexport) int32_t Read(void* const session_handle, int32_t timeout, char* response, size_t* const size);

// Release resources allocated by the session.
__declspec(dllexport) int32_t Close(void* const session_handle);

// Get error information.
__declspec(dllexport) int32_t GetError(void* const session_handle, int32_t* const code, char* const description, size_t* const size);

#ifdef __cplusplus
}
#endif
