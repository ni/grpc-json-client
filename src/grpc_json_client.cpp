#include "grpc_json_client.h"

#include "error_code.h"
#include "exceptions.h"
#include "session.h"
#include "unary_unary_json_client.h"

using grpc::ChannelCredentials;
using ni::grpc_json_client::ErrorCode;
using ni::grpc_json_client::JsonClientException;
using ni::grpc_json_client::Session;
using ni::grpc_json_client::UnaryUnaryJsonClient;
using std::lock_guard;
using std::mutex;
using std::shared_ptr;
using std::string;

// Helper function for locking access to the session and catching exceptions.
int32_t Evaluate(void* const session_handle, const std::function<void(UnaryUnaryJsonClient&)>& func)
{
    Session* session = (Session*)session_handle;
    lock_guard<mutex> _lock(session->lock());
    session->ClearLastException();
    try
    {
        func(session->client());
    }
    catch (JsonClientException& ex)
    {
        session->last_exception() = ex;
    }
    catch (...)
    {
        session->last_exception() = JsonClientException("An unhandled exception occurred.");
    }
    return (int32_t)session->last_error_code();
}

int32_t InitInsecure(const char* target, void** const session_handle)
{
    shared_ptr<ChannelCredentials> credentials = grpc::InsecureChannelCredentials();
    *session_handle = new Session(target, credentials);
    return Evaluate(session_handle, [](UnaryUnaryJsonClient& client) { client.QueryReflectionService(); });
}

int32_t Write(void* const session_handle, const char* service, const char* method, const char* request)
{
    return Evaluate(session_handle, [=](UnaryUnaryJsonClient& client) { client.Write(service, method, request); });
}

int32_t Read(void* const session_handle, char* buffer, size_t* const size)
{
    string response;
    int32_t error_code = Evaluate(session_handle, [&](UnaryUnaryJsonClient& client) { response = client.Read(); });
    if (error_code >= 0)
    {
        if (buffer != nullptr)
        {
            response.copy(buffer, *size, 0);
        }
        else if (size != nullptr)
        {
            *size = response.size();
        }
    }
    return error_code;
}

int32_t Close(void* const session_handle)
{
    delete session_handle;
    return (int32_t)ErrorCode::kNone;
}

int32_t GetError(void* const session_handle, int32_t* const code, char* const description, size_t* const size)
{
    Session* session = (Session*)session_handle;
    string last_error_description;
    if (session == nullptr)
    {
        last_error_description = ni::grpc_json_client::GetErrorDescription((ErrorCode)*code);
    }
    else
    {
        lock_guard<mutex> lock(session->lock());
        last_error_description = session->last_error_description();
        if (code != nullptr)
        {
            *code = (int32_t)session->last_error_code();
        }
    }
    if (description != nullptr)
    {
        last_error_description.copy(description, *size, 0);
    }
    else if (size != nullptr)
    {
        *size = last_error_description.size();
    }
    return (int32_t)ErrorCode::kNone;
}
