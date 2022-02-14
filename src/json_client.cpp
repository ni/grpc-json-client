#include "json_client.h"

#include "error_code.h"
#include "exceptions.h"
#include "session.h"
#include "unary_unary_json_client.h"

using grpc::ChannelCredentials;
using ni::json_client::ErrorCode;
using ni::json_client::JsonClientException;
using ni::json_client::Session;
using ni::json_client::UnaryUnaryJsonClient;
using std::lock_guard;
using std::mutex;
using std::shared_ptr;
using std::string;

// Helper function for locking access to the session and catching exceptions.
ErrorCode Evaluate(Session* const session, const std::function<void(UnaryUnaryJsonClient&)>& func)
{
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
    return session->last_error_code();
}

int32_t InitInsecure(const char* target, void** const session_handle)
{
    shared_ptr<ChannelCredentials> credentials = grpc::InsecureChannelCredentials();
    Session* session = new Session(target, credentials);
    *session_handle = session;
    return Evaluate(session, [](UnaryUnaryJsonClient& client) { client.QueryReflectionService(); });
}

int32_t Write(void* const session_handle, const char* service, const char* method, const char* request)
{
    return Evaluate((Session*)session_handle, [=](UnaryUnaryJsonClient& client) { client.Write(service, method, request); });
}

int32_t Read(void* const session_handle, char* buffer, size_t* const size)
{
    const string* response = nullptr;
    ErrorCode error_code = Evaluate((Session*)session_handle, [&](UnaryUnaryJsonClient& client) { response = client.Read(); });
    if (error_code < 0)
    {
        return error_code;
    }
    else if (buffer != nullptr)
    {
        response->copy(buffer, *size, 0);
    }
    else if (size != nullptr)
    {
        *size = response->size();
    }
    return error_code;
}

int32_t Close(void* const session_handle)
{
    delete session_handle;
    return 0;
}

int32_t GetErrorMessage(int32_t error_code, char* const message, size_t* const size)
{
    string error_message;
    switch (error_code)
    {
    case ErrorCode::NONE:
        break;
    case ErrorCode::SERVICE_NOT_FOUND:
        error_message = "Service not found";
        break;
    case ErrorCode::METHOD_NOT_FOUND:
        error_message = "Method not found";
        break;
    default:
        error_message = "Error code is undefined";
    }

    if (message != nullptr)
    {
        error_message.copy(message, *size, 0);
    }
    else if (size != nullptr)
    {
        *size = error_message.size();
    }
    return 0;
}
