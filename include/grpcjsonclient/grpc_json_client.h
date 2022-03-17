
#pragma once

#include <stdint.h>

#define DLLEXPORT __declspec(dllexport)

#ifdef __cplusplus
extern "C" {
#endif

// Initializes a session to the target with an insecure channel. The target is typically the URI of
// the host. For example: "localhost:50051" or "server.domain.com:50051".
DLLEXPORT int32_t GrpcJsonClient_Initialize(const char* target, intptr_t* session_handle);

// Resets the descriptor database to it's default state. Use this function to force the session to
// request new file descriptors for services.
DLLEXPORT int32_t GrpcJsonClient_ResetDescriptorDatabase(intptr_t session_handle);

// Queries the host for the file descriptors of all services exposed by the reflection service.
DLLEXPORT int32_t GrpcJsonClient_FillDescriptorDatabase(intptr_t session_handle, int32_t timeout);

// Starts an asynchronous call to a unary unary RPC. The returned tag is a unique identifier that
// should be passed to GrpcJsonClient_FinishAsyncCall to complete the call. The service argument
// must be the fully-qualified service name, for example: "package.subpackage.service".
DLLEXPORT int32_t GrpcJsonClient_StartAsyncCall(
    intptr_t session_handle,
    const char* service,
    const char* method,
    const char* request,
    int32_t timeout,
    intptr_t* tag);

// Finishes an asynchronous call started by GrpcJsonClient_StartAsyncCall. Pass null as the response
// argument to query the function for the minimum buffer size.
DLLEXPORT int32_t GrpcJsonClient_FinishAsyncCall(
    intptr_t session_handle, intptr_t tag, int32_t timeout, char* response, size_t* size);

// Performs a blocking unary unary RPC call. This function is equivalent to calling
// GrpcJsonClient_StartAsyncCall followed by GrpcJsonClient_FinishAsyncCall but with a global
// timeout. Initialize the tag argument to zero on the first call to this function. The returned tag
// is a unique identifier that should be passed to sebsequent calls to this function. See
// GrpcJsonClient_StartAsyncCall and GrpcJsonClient_FinishAsyncCall for additional parameter
// semantics.
DLLEXPORT int32_t GrpcJsonClient_BlockingCall(
    intptr_t session_handle,
    const char* service,
    const char* method,
    const char* request,
    int32_t timeout,
    intptr_t* tag,
    char* response,
    size_t* size);

// Locks the session for exclusive use by the calling thread. Use this function to prevent other
// threads from accessing the session simultaneously, potentially causing race conditions. Threads
// that do not own the session will block until the specified timeout is reached. The has_lock
// argument is set to true if the thread successfully acquires the lock; otherwise it is set to
// false. The lock is recursive.
DLLEXPORT int32_t GrpcJsonClient_LockSession(
    intptr_t session_handle, int32_t timeout, uint8_t* has_lock);

// Unlocks the session. Call this method the same number of times as GrpcJsonClient_LockSession to
// prevent deadlock.
DLLEXPORT int32_t GrpcJsonClient_UnlockSession(intptr_t session_handle);

// Gets and clears error information from the session. Sessions store the most recent error or
// warning that occures. Pass null as the buffer argument to query the function for the minimum
// buffer size.
DLLEXPORT int32_t GrpcJsonClient_GetError(
    intptr_t session_handle, int32_t* code, char* buffer, size_t* size);

// Gets the corresponding generic error message for the specified error code. Pass null as the
// session_handle argument to prevent the session from updating it's error state should an error
// occur while calling this function. GrpcJsonClient_GetError should generally be preferred over
// this function since it provides context specific error messages. Pass null as the buffer
// argument to query the function for the minimum buffer size.
DLLEXPORT int32_t GrpcJsonClient_GetErrorString(
    intptr_t session_handle, int32_t code, char* buffer, size_t* size);

// Releases resources allocated by the session. Any active RPCs are implicitly cancelled.
DLLEXPORT int32_t GrpcJsonClient_Close(intptr_t session_handle);

#ifdef __cplusplus
}
#endif
