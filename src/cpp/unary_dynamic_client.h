#pragma once

#include "dynamic_client.h"
#include <grpcpp/generic/generic_stub.h>

namespace ni
{
	class UnaryDynamicClient : DynamicClient
	{
	private:
		std::shared_ptr<grpc::GenericStub> _stub;
		std::shared_ptr<grpc::ClientContext> _context;
		std::shared_ptr<grpc::CompletionQueue> _completion_queue;
		const google::protobuf::MethodDescriptor* _method_type = nullptr;
		std::unique_ptr<grpc::GenericClientAsyncResponseReader> _response_reader;
		std::string _response;

	public:
		UnaryDynamicClient(const std::string& target);

		void Write(const std::string& service_name, const std::string& method_name, const std::string& request_json);
		const std::string* Read();
	};
}
