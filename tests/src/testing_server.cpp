#include "testing_server.h"

using grpc::ServerContext;
using grpc::Status;

namespace ni
{
	namespace grpc_json_client
	{
		Status TestingServiceImpl::UnaryUnaryEcho(ServerContext* context, const UnaryUnaryEchoMessage* request, UnaryUnaryEchoMessage* response)
		{
			*response = *request;
			return Status::OK;
		}
	}
}
