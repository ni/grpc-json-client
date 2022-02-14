#pragma once

#include <grpcpp/grpcpp.h>

#include "exceptions.h"
#include "unary_unary_dynamic_client.h"

namespace ni
{
	class Session
	{
	public:
		Session(const std::string& target, const grpc::ChannelCredentials& credentials);

		UnaryUnaryDynamicClient& client();
		int last_error_code();
		string last_error_description();

	private:
		UnaryUnaryDynamicClient _client;
		DynamicClientException _last_exception;
	};
}
