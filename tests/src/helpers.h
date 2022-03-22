#pragma once

#include <cstdint>
#include <string>

namespace ni {
namespace grpc_json_client {

int32_t FinishAsyncCallHelper(
    intptr_t session,
    intptr_t tag,
    int32_t timeout,
    std::string* response
);
int32_t BlockingCallHelper(
    intptr_t session,
    const std::string& service,
    const std::string& method,
    const std::string& request,
    int32_t timeout,
    std::string* response);
int32_t GetDefaultRequestHelper(
    intptr_t session,
    const std::string& service,
    const std::string& method,
    int32_t timeout,
    std::string* request);
int32_t GetErrorHelper(intptr_t session, int32_t* code, std::string* message);
int32_t GetErrorStringHelper(intptr_t session, int32_t code, std::string* message);
void CheckGetErrorHelper(
    intptr_t session,
    int32_t expected_code,
    const std::string& message_core,
    bool starts_with = false);
void CheckGetErrorStringHelper(
    intptr_t session, int32_t code, const std::string& message_core, bool starts_with = false);

}  // namespace grpc_json_client
}  // namespace ni
