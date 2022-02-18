#include "session.h"

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

        int32_t Session::Write(const char* service, const char* method, const char* request)
        {
            return Evaluate([=](UnaryUnaryJsonClient& client) { client.Write(service, method, request); });
        }

        int32_t Session::Read(int32_t timeout, char* buffer, size_t* const size)
        {
            int32_t error_code = 0;
            if (!_last_response)
            {
                error_code = Evaluate([&](UnaryUnaryJsonClient& client) { _last_response = std::make_unique<string>(client.Read(timeout)); });
            }
            if (error_code >= 0)
            {
                if (buffer != nullptr)
                {
                    strncpy(buffer, _last_response->c_str(), *size);
                    _last_response.reset();
                }
                else if (size != nullptr)
                {
                    *size = _last_response->size() + 1;  // include null character
                }
            }
            return error_code;
        }

        int32_t Session::Close()
        {
            // placeholder for any pre-destructor cleanup operations
            return static_cast<int32_t>(ErrorCode::kNone);
        }

        int32_t Session::GetError(Session* const session, int32_t* const code, char* const description, size_t* const size)
        {
            string last_error_description;
            if (session == nullptr)
            {
                last_error_description = GetErrorDescription((ErrorCode)*code);
            }
            else
            {
                lock_guard<mutex> lock(session->_lock);
                last_error_description = session->_last_error_description();
                if (code != nullptr)
                {
                    *code = session->_last_error_code();
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

        int32_t Session::_last_error_code() const
        {
            ErrorCode error_code = _last_exception ? _last_exception->error_code() : ErrorCode::kNone;
            return static_cast<int32_t>(error_code);
        }

        string Session::_last_error_description() const
        {
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
            return _last_error_code();
        }
	}
}