
#pragma once

#include <stdint.h>

#define DLLEXPORT __declspec(dllexport)

#ifdef __cplusplus
extern "C" {
#endif

// Initializes a session with an insecure channel.
DLLEXPORT int32_t GrpcJsonClient_Initialize(const char* target, intptr_t* session_handle);

// Resets the descriptor database to it's default state.
DLLEXPORT int32_t GrpcJsonClient_ResetDescriptorDatabase(intptr_t session_handle);

// Queries the reflection service for file descriptors.
DLLEXPORT int32_t GrpcJsonClient_FillDescriptorDatabase(intptr_t session_handle, int32_t timeout);

// Starts an asynchronous call.
DLLEXPORT int32_t GrpcJsonClient_StartAsyncCall(
    intptr_t session_handle,
    const char* service,
    const char* method,
    const char* request,
    int32_t timeout,
    intptr_t* tag);

// Finishes an asynchronous call.
DLLEXPORT int32_t GrpcJsonClient_FinishAsyncCall(
    intptr_t session_handle, intptr_t tag, int32_t timeout, char* response, size_t* size);

// Performs a blocking call.
DLLEXPORT int32_t GrpcJsonClient_BlockingCall(
    intptr_t session_handle,
    const char* service,
    const char* method,
    const char* request,
    int32_t timeout,
    intptr_t* tag,
    char* response,
    size_t* size);

// Locks the session.
DLLEXPORT int32_t GrpcJsonClient_LockSession(intptr_t session_handle);

// Unlocks the session.
DLLEXPORT int32_t GrpcJsonClient_UnlockSession(intptr_t session_handle);

// Gets error information.
DLLEXPORT int32_t GrpcJsonClient_GetError(
    intptr_t session_handle, int32_t* code, char* buffer, size_t* size);

// Gets error information.
DLLEXPORT int32_t GrpcJsonClient_GetErrorString(
    intptr_t session_handle, int32_t code, char* buffer, size_t* size);

// Releases resources allocated by the session.
DLLEXPORT int32_t GrpcJsonClient_Close(intptr_t session_handle);

#ifdef __cplusplus
}
#endif
