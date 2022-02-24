#include "testing_server.h"

#include <chrono>
#include <thread>

#include <grpcpp/ext/proto_server_reflection_plugin.h>

using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;
using std::string;
using std::chrono::milliseconds;

namespace ni
{
	namespace grpc_json_client
	{
		Status TestingServiceImpl::UnaryUnaryEcho(ServerContext* context, const UnaryUnaryEchoMessage* request, UnaryUnaryEchoMessage* response)
		{
			std::this_thread::sleep_for(milliseconds(request->delay()));
			*response = *request;
			return Status::OK;
		}

		TestingServer::TestingServer(const string& address) :
			_address(address)
		{}

		void TestingServer::StartInsecure()
		{
			grpc::reflection::InitProtoReflectionServerBuilderPlugin();
			ServerBuilder builder;
			builder.AddListeningPort(_address, grpc::InsecureServerCredentials());
			builder.RegisterService(&_service);
			_server = builder.BuildAndStart();
		}

		void TestingServer::Stop()
		{
			_server->Shutdown();
			_server->Wait();
		}
	}
}
