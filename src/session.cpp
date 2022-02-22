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
            _error_occurred(false),
            _last_error_code(ErrorCode::kNone)
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
                *code = session->last_error_code();
                description = session->last_error_description();
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

        int32_t Session::last_error_code()
        {
            return static_cast<int32_t>(_error_occurred ? _last_error_code : ErrorCode::kNone);
        }

        const string& Session::last_error_description()
        {
            return _error_occurred ? _last_error_description : "";
        }

        int32_t Session::Evaluate(const function<void(UnaryUnaryJsonClient&)>& func)
        {
            lock_guard<recursive_mutex> lock(_lock);
            _error_occurred = false;
            try
            {
                func(_client);
            }
            catch (const JsonClientException& ex)
            {
                _error_occurred = true;
                _last_error_code = ex.error_code();
                _last_error_description = ex.what();
            }
            catch (const exception& ex)
            {
                _error_occurred = true;
                _last_error_code = ErrorCode::kUnknownError;
                _last_error_description = "An unhandled exception occurred.\n\n";
                _last_error_description += ex.what();
            }
            catch (...)
            {
                _error_occurred = true;
                _last_error_code = ErrorCode::kUnknownError;
                _last_error_description = "An unhandled exception occurred.\n\n";
            }
            return last_error_code();
        }
    }
}