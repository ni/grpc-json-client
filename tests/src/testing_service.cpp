
#include "testing_service.h"

#include <chrono>
#include <thread>

#include <grpcpp/grpcpp.h>

using grpc::ServerContext;
using grpc::Status;
using std::chrono::milliseconds;

namespace ni {
namespace grpc_json_client {

Status TestingServiceImpl::UnaryUnaryEcho(
    ServerContext* context, const UnaryUnaryEchoMessage* request, UnaryUnaryEchoMessage* response
) {
    std::this_thread::sleep_for(milliseconds(request->delay()));
    *response = *request;
    return Status::OK;
}

}  // namespace grpc_json_client
}  // namespace ni
