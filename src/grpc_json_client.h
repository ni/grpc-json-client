#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

// Initialize a session with an insecure channel.
__declspec(dllexport) int32_t InitInsecure(const char* target, void** session_handle);

// Start a call to the host.
__declspec(dllexport) int32_t StartAsyncCall(void* session_handle, const char* service, const char* method, const char* request, void** tag);

// Finish a call to the host.
__declspec(dllexport) int32_t FinishAsyncCall(void* session_handle, void* tag, int32_t timeout, char* response, size_t* size);

// Lock the session.
__declspec(dllexport) int32_t LockSession(void* session_handle);

// Unlock the session.
__declspec(dllexport) int32_t UnlockSession(void* session_handle);

// Release resources allocated by the session.
__declspec(dllexport) int32_t Close(void* session_handle);

// Get error information.
__declspec(dllexport) int32_t GetError(void* session_handle, int32_t* code, char* description, size_t* size);

#ifdef __cplusplus
}
#endif
