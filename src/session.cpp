#include "session.h"

#include <mutex>

#include "error_code.h"

using grpc::ChannelCredentials;
using std::exception;
using std::function;
using std::lock_guard;
using std::mutex;
using std::shared_ptr;
using std::string;

namespace ni
{
    namespace grpc_json_client
    {
        Session::Session(const string& target, const shared_ptr<ChannelCredentials>& credentials) :
            _client(target, credentials),
            _last_exception(nullptr)
        {}

        int32_t Session::Init()
        {
            return Evaluate([](UnaryUnaryJsonClient& client) { client.QueryReflectionService(); });
        }

        int32_t Session::StartAsyncCall(const char* service, const char* method, const char* request, void** tag)
        {
            return Evaluate([=](UnaryUnaryJsonClient& client) { *tag = client.StartAsyncCall(service, method, request); });
        }

        int32_t Session::FinishAsyncCall(void* tag, int32_t timeout, char* buffer, size_t* size)
        {
            return Evaluate(
                [=](UnaryUnaryJsonClient& client)
                {
                    if (!_responses.count(tag))
                    {
                        _responses[tag] = client.FinishAsyncCall(tag, timeout);
                    }
                    string& response = _responses[tag];
                    if (buffer == nullptr)
                    {
                        *size = response.size() + 1;  // include null character
                    }
                    else if (*size >= response.size())
                    {
                        strncpy(buffer, response.c_str(), *size);
                        _responses.erase(tag);
                    }
                    else
                    {
                        throw BufferSizeOutOfRangeException("Buffer size is not large enough for the response.");
                    }
                }
            );
        }

        int32_t Session::Close()
        {
            // placeholder for any pre-destructor cleanup operations
            return static_cast<int32_t>(ErrorCode::kNone);
        }

        int32_t Session::GetError(Session* session, int32_t* code, char* description, size_t* size)
        {
            string last_error_description;
            if (session == nullptr)
            {
                last_error_description = GetErrorDescription((ErrorCode)*code);
            }
            else
            {
                last_error_description = session->last_error_description();
                if (code != nullptr)
                {
                    *code = session->last_error_code();
                }
            }
            if (description != nullptr)
            {
                strncpy(description, last_error_description.c_str(), *size);
            }
            else if (size != nullptr)
            {
                *size = last_error_description.size() + 1;  // include null character
            }
            return static_cast<int32_t>(ErrorCode::kNone);
        }

        int32_t Session::last_error_code() const
        {
            lock_guard<mutex> lock(_lock);
            ErrorCode error_code = _last_exception ? _last_exception->error_code() : ErrorCode::kNone;
            return static_cast<int32_t>(error_code);
        }

        string Session::last_error_description() const
        {
            lock_guard<mutex> lock(_lock);
            return _last_exception ? _last_exception->what() : "";
        }

        int32_t Session::Evaluate(const function<void(UnaryUnaryJsonClient&)>& func)
        {
            lock_guard<mutex> lock(_lock);
            _last_exception.reset();
            try
            {
                func(_client);
            }
            catch (const JsonClientException& json_client_ex)
            {
                _last_exception = std::make_unique<JsonClientException>(json_client_ex);
            }
            catch (const exception& std_ex)
            {
                string summary("An unhandled exception occurred.\n\n");
                JsonClientException json_client_ex(summary + std_ex.what());
                _last_exception = std::make_unique<JsonClientException>(json_client_ex);
            }
            catch (...)
            {
                JsonClientException json_client_ex("An unhandled exception occurred.");
                _last_exception = std::make_unique<JsonClientException>(json_client_ex);
            }
            return _last_exception ? _last_exception->error_code() : ErrorCode::kNone;
        }
    }
}