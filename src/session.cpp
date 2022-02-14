#include "session.h"

using grpc::ChannelCredentials;
using std::string;

namespace ni
{
	Session::Session(const string& target, const ChannelCredentials& credentials) :
		_client(target, credentials)
	{
	}

	UnaryUnaryDynamicClient& Session::client()
	{
		return _client;
	}

	DynamicClientException& Session::last_error_code()
	{
		return _last_exception.error_code();
	}
}
