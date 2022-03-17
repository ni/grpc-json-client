
#include <grpcjsonclient/grpc_json_client.h>

#include <chrono>
#include <cstdint>
#include <memory>

#include "grpcpp/grpcpp.h"

#include "session.h"

using grpc::ChannelCredentials;
using ni::grpc_json_client::Session;
using std::chrono::milliseconds;
using std::chrono::system_clock;
using std::shared_ptr;
using std::unique_ptr;

// Converts a timeout to a deadline.
system_clock::time_point DeadlineFromTimeout(int32_t timeout) {
    if (timeout < 0) {
        return system_clock::time_point::max();
    }
    return system_clock::now() + milliseconds(timeout);
}

int32_t GrpcJsonClient_Initialize(const char* target, intptr_t* session_handle) {
    shared_ptr<ChannelCredentials> credentials = grpc::InsecureChannelCredentials();
    *session_handle = reinterpret_cast<intptr_t>(new Session(target, credentials));
    return 0;
}

int32_t GrpcJsonClient_ResetDescriptorDatabase(intptr_t session_handle) {
    Session* session = reinterpret_cast<Session*>(session_handle);
    return session->ResetDescriptorDatabase();
}

int32_t GrpcJsonClient_FillDescriptorDatabase(intptr_t session_handle, int32_t timeout) {
    Session* session = reinterpret_cast<Session*>(session_handle);
    system_clock::time_point deadline = DeadlineFromTimeout(timeout);
    return session->FillDescriptorDatabase(deadline);
}

int32_t GrpcJsonClient_StartAsyncCall(
    intptr_t session_handle,
    const char* service,
    const char* method,
    const char* request,
    int32_t timeout,
    intptr_t* tag
) {
    Session* session = reinterpret_cast<Session*>(session_handle);
    system_clock::time_point deadline = DeadlineFromTimeout(timeout);
    void** reinterpreted_tag = reinterpret_cast<void**>(tag);
    return session->StartAsyncCall(service, method, request, deadline, reinterpreted_tag);
}

int32_t GrpcJsonClient_FinishAsyncCall(
    intptr_t session_handle, intptr_t tag, int32_t timeout, char* buffer, size_t* size
) {
    Session* session = reinterpret_cast<Session*>(session_handle);
    void* reinterpreted_tag = reinterpret_cast<void*>(tag);
    system_clock::time_point deadline = DeadlineFromTimeout(timeout);
    return session->FinishAsyncCall(reinterpreted_tag, deadline, buffer, size);
}

int32_t GrpcJsonClient_BlockingCall(
    intptr_t session_handle,
    const char* service,
    const char* method,
    const char* request,
    int32_t timeout,
    intptr_t* tag,
    char* response,
    size_t* size
) {
    Session* session = reinterpret_cast<Session*>(session_handle);
    system_clock::time_point deadline = DeadlineFromTimeout(timeout);
    void** reinterpreted_tag = reinterpret_cast<void**>(tag);
    return session->BlockingCall(
        service, method, request, deadline, reinterpreted_tag, response, size);
}

int32_t GrpcJsonClient_LockSession(intptr_t session_handle, int32_t timeout, uint8_t* has_lock) {
    Session* session = reinterpret_cast<Session*>(session_handle);
    system_clock::time_point deadline = DeadlineFromTimeout(timeout);
    return session->Lock(deadline, has_lock);
}

int32_t GrpcJsonClient_UnlockSession(intptr_t session_handle) {
    Session* session = reinterpret_cast<Session*>(session_handle);
    return session->Unlock();
}

int32_t GrpcJsonClient_GetDefaultRequest(
    intptr_t session_handle,
    const char* service,
    const char* method,
    int32_t timeout,
    char* buffer,
    size_t* size
) {
    Session* session = reinterpret_cast<Session*>(session_handle);
    system_clock::time_point deadline = DeadlineFromTimeout(timeout);
    return session->GetDefaultRequest(service, method, deadline, buffer, size);
}

int32_t GrpcJsonClient_GetError(
    intptr_t session_handle, int32_t* code, char* buffer, size_t* size
) {
    Session* session = reinterpret_cast<Session*>(session_handle);
    return session->GetError(code, buffer, size);
}

int32_t GrpcJsonClient_GetErrorString(
    intptr_t session_handle, int32_t code, char* buffer, size_t* size
) {
    Session* session = reinterpret_cast<Session*>(session_handle);
    return Session::GetErrorString(session, code, buffer, size);
}

int32_t GrpcJsonClient_Close(intptr_t session_handle) {
    unique_ptr<Session> session(reinterpret_cast<Session*>(session_handle));
    int32_t error_code = session->Close();
    return error_code;
}
