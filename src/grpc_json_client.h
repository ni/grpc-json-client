
#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Initializes a session with an insecure channel.
__declspec(dllexport) int32_t GrpcJsonClient_Initialize(
    const char* target, void** session_handle
);

// Resets the descriptor database to it's default state.
__declspec(dllexport) int32_t GrpcJsonClient_ResetDescriptorDatabase(void* session_handle);

// Queries the reflection service for file descriptors.
__declspec(dllexport) int32_t GrpcJsonClient_FillDescriptorDatabase(void* session_handle);

// Starts an asynchronous call.
__declspec(dllexport) int32_t GrpcJsonClient_StartAsyncCall(
    void* session_handle, const char* service, const char* method, const char* request, void** tag
);

// Finishes an asynchronous call.
__declspec(dllexport) int32_t GrpcJsonClient_FinishAsyncCall(
    void* session_handle, void* tag, int32_t timeout, char* response, size_t* size
);

// Performs a blocking call.
__declspec(dllexport) int32_t GrpcJsonClient_BlockingCall(
    void* session_handle,
    const char* service,
    const char* method,
    const char* request,
    void** tag,
    int32_t timeout,
    char* response,
    size_t* size
);

// Locks the session.
__declspec(dllexport) int32_t GrpcJsonClient_LockSession(void* session_handle);

// Unlocks the session.
__declspec(dllexport) int32_t GrpcJsonClient_UnlockSession(void* session_handle);

// Gets error information.
__declspec(dllexport) int32_t GrpcJsonClient_GetError(
    void* session_handle, int32_t* code, char* buffer, size_t* size
);

// Gets error information.
__declspec(dllexport) int32_t GrpcJsonClient_GetErrorString(
    void* session_handle, int32_t code, char* buffer, size_t* size
);

// Releases resources allocated by the session.
__declspec(dllexport) int32_t GrpcJsonClient_Close(void* session_handle);

#ifdef __cplusplus
}
#endif
