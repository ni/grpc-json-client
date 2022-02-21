#include "session.h"

#include "error_code.h"

using grpc::ChannelCredentials;
using std::exception;
using std::function;
using std::lock_guard;
using std::recursive_mutex;
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

        int32_t Session::QueryReflectionService()
        {
            return Evaluate(
                [](UnaryUnaryJsonClient& client)
                { 
                    client.QueryReflectionService(); 
                }
            );
        }

        int32_t Session::StartAsyncCall(const char* service, const char* method, const char* request, void** tag)
        {
            return Evaluate(
                [=](UnaryUnaryJsonClient& client)
                { 
                    *tag = client.StartAsyncCall(service, method, request); 
                }
            );
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
                    const string& response = _responses[tag];
                    if (!buffer)
                    {
                        *size = response.size() + 1;  // include null char
                    }
                    else if (*size > response.size())  // null char
                    {
                        strncpy(buffer, response.c_str(), *size);
                        _responses.erase(tag);
                    }
                    else
                    {
                        throw BufferSizeOutOfRangeException(
                            "Buffer size is too small to accommodate the response."
                        );
                    }
                }
            );
        }

        int32_t Session::Lock()
        {
            return Evaluate(
                [&](const UnaryUnaryJsonClient&)
                {
                    _lock.lock();
                }
            );
        }

        int32_t Session::Unlock()
        {
            return Evaluate(
                [&](const UnaryUnaryJsonClient&)
                {
                    _lock.unlock();
                }
            );
        }

        int32_t Session::Close()
        {
            // placeholder for any pre-destructor cleanup operations
            return static_cast<int32_t>(ErrorCode::kNone);
        }

        int32_t Session::GetError(Session* session, int32_t* code, char* buffer, size_t* size)
        {
            string description;
            if (session)
            {
                lock_guard<recursive_mutex> lock(session->_lock);
                *code = session->_last_error_code();
                description = session->_last_error_description();
            }
            else
            {
                description = GetErrorDescription(static_cast<ErrorCode>(*code));
            }
            if (buffer)
            {
                strncpy(buffer, description.c_str(), *size);
                if (*size <= description.size())
                {
                    buffer[*size - 1] = NULL;  // strncpy doesn't add null char
                    return static_cast<int32_t>(ErrorCode::kBufferSizeOutOfRangeWarning);
                }
            }
            else
            {
                *size = description.size() + 1;  // include null char
            }
            return static_cast<int32_t>(ErrorCode::kNone);
        }

        int32_t Session::_last_error_code()
        {
            return static_cast<int32_t>(
                _last_exception ? _last_exception->error_code() : ErrorCode::kNone
            );
        }

        string Session::_last_error_description()
        {
            return _last_exception ? _last_exception->what() : "";
        }

        int32_t Session::Evaluate(const function<void(UnaryUnaryJsonClient&)>& func)
        {
            lock_guard<recursive_mutex> lock(_lock);
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
            return _last_error_code();
        }
    }
}