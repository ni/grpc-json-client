#include "session.h"

using grpc::ChannelCredentials;
using std::mutex;
using std::string;

namespace ni
{
	namespace grpc_json_client
	{
		Session::Session(const std::string& target, const std::shared_ptr<grpc::ChannelCredentials>& credentials) :
			_unary_unary_client(target, credentials),
			_last_exception(nullptr)
		{}

		UnaryUnaryJsonClient& Session::client()
		{
			return _unary_unary_client;
		}

		mutex& Session::lock()
		{
			return _lock;
		}

		JsonClientException& Session::last_exception()
		{
			return *_last_exception;
		}

		ErrorCode Session::last_error_code() const
		{
			return _last_exception ? _last_exception->error_code() : ErrorCode::kNone;
		}

		string Session::last_error_description() const
		{
			return _last_exception ? _last_exception->what() : "";
		}

		void Session::ClearLastException()
		{
			_last_exception.reset();
		}
	}
}
