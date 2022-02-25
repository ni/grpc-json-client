
#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Initialize a session with an insecure channel.
__declspec(dllexport) int32_t GrpcJsonClient_InitInsecure(
    const char* target, void** session_handle
);

// Start a call to the host.
__declspec(dllexport) int32_t GrpcJsonClient_StartAsyncCall(
    void* session_handle, const char* service, const char* method, const char* request, void** tag
);

// Finish a call to the host.
__declspec(dllexport) int32_t GrpcJsonClient_FinishAsyncCall(
    void* session_handle, void* tag, int32_t timeout, char* response, size_t* size
);

// Lock the session.
__declspec(dllexport) int32_t GrpcJsonClient_LockSession(void* session_handle);

// Unlock the session.
__declspec(dllexport) int32_t GrpcJsonClient_UnlockSession(void* session_handle);

// Release resources allocated by the session.
__declspec(dllexport) int32_t GrpcJsonClient_Close(void* session_handle);

// Get error information.
__declspec(dllexport) int32_t GrpcJsonClient_GetError(
    void* session_handle, int32_t* code, char* buffer, size_t* size
);

#ifdef __cplusplus
}
#endif
