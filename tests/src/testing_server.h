#pragma once

#include <string>
#include <memory>

#include <grpcpp/grpcpp.h>

#include "testing_service.grpc.pb.h"

namespace ni
{
	namespace grpc_json_client
	{
		class TestingServiceImpl final : public TestingService::Service
		{
			grpc::Status UnaryUnaryEcho(grpc::ServerContext* context, const UnaryUnaryEchoMessage* request, UnaryUnaryEchoMessage* response) override;
		};

		class TestingServer
		{
		private:
			std::string _address;
			TestingServiceImpl _service;
			std::unique_ptr<grpc::Server> _server;

		public:
			TestingServer(const std::string& address);
			void StartInsecure();
			void Stop();
		};
	}
}
