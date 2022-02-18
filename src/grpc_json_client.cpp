#include "grpc_json_client.h"

#include "session.h"

using grpc::ChannelCredentials;
using ni::grpc_json_client::Session;
using std::shared_ptr;

int32_t InitInsecure(const char* target, void** const session_handle)
{
    shared_ptr<ChannelCredentials> credentials = grpc::InsecureChannelCredentials();
    Session* session = new Session(target, credentials);
    *session_handle = session;
    return session->Init();
}

int32_t Write(void* const session_handle, const char* service, const char* method, const char* request)
{
    return ((Session*)session_handle)->Write(service, method, request);
}

int32_t Read(void* const session_handle, int32_t timeout, char* buffer, size_t* const size)
{
    return ((Session*)session_handle)->Read(timeout, buffer, size);
}

int32_t Close(void* const session_handle)
{
    int32_t error_code = ((Session*)session_handle)->Close();
    delete session_handle;
    return error_code;
}

int32_t GetError(void* const session_handle, int32_t* const code, char* const description, size_t* const size)
{
    return Session::GetError((Session*)session_handle, code, description, size);
}
