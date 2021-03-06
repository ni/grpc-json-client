
#include "helpers.h"

#include <cstdint>
#include <cstdio>
#include <memory>
#include <string>

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "grpcjsonclient/grpc_json_client.h"

using std::string;
using std::unique_ptr;

namespace ni {
namespace grpc_json_client {

int32_t FinishAsyncCallHelper(
    intptr_t session,
    intptr_t tag,
    int32_t timeout,
    string* response
) {
    size_t size = 0;
    int32_t error_code = GrpcJsonClient_FinishAsyncCall(session, tag, timeout, nullptr, &size);
    if (error_code < 0) {
        return error_code;
    }
    unique_ptr<char> buffer(new char[size]);
    int32_t next_error_code = GrpcJsonClient_FinishAsyncCall(session, tag, 0, buffer.get(), &size);
    if (next_error_code < 0) {
        return next_error_code;
    }
    if (response) {
        *response = buffer.get();
    }
    return error_code > 0 ? error_code : next_error_code;
}

int32_t BlockingCallHelper(
    intptr_t session,
    const string& service,
    const string& method,
    const string& request,
    int32_t timeout,
    string* response
) {
    intptr_t tag = 0;
    size_t size = 0;
    int32_t error_code = GrpcJsonClient_BlockingCall(
        session, service.c_str(), method.c_str(), request.c_str(), timeout, &tag, nullptr, &size);
    if (error_code < 0) {
        return error_code;
    }
    unique_ptr<char> buffer(new char[size]);
    int32_t next_error_code = GrpcJsonClient_BlockingCall(
        session, nullptr, nullptr, nullptr, 0, &tag, buffer.get(), &size);
    if (next_error_code < 0) {
        return next_error_code;
    }
    if (response) {
        *response = buffer.get();
    }
    return error_code > 0 ? error_code : next_error_code;
}

int32_t GetDefaultRequestHelper(
    intptr_t session, const string& service, const string& method, int32_t timeout, string* request
) {
    size_t size = 0;
    int32_t error_code = GrpcJsonClient_GetDefaultRequest(
        session, service.c_str(), method.c_str(), timeout, nullptr, &size);
    if (error_code < 0) {
        return error_code;
    }
    unique_ptr<char> buffer(new char[size]);
    int32_t next_error_code = GrpcJsonClient_GetDefaultRequest(
        session, service.c_str(), method.c_str(), timeout, buffer.get(), &size);
    if (next_error_code < 0) {
        return next_error_code;
    }
    if (request) {
        *request = buffer.get();
    }
    return error_code > 0 ? error_code : next_error_code;
}

int32_t GetErrorHelper(intptr_t session, int32_t* code, string* message) {
    size_t size = 0;
    int32_t error_code = GrpcJsonClient_GetError(session, nullptr, nullptr, &size);
    if (error_code < 0) {
        return error_code;
    }
    int32_t queried_code = 0;
    unique_ptr<char> buffer(new char[size]);
    int32_t next_error_code = GrpcJsonClient_GetError(session, &queried_code, buffer.get(), &size);
    if (next_error_code < 0) {
        return next_error_code;
    }
    if (code) {
        *code = queried_code;
    }
    if (message) {
        *message = buffer.get();
    }
    return error_code > 0 ? error_code : next_error_code;
}

int32_t GetErrorStringHelper(intptr_t session, int32_t code, string* message) {
    size_t size;
    int32_t error_code = GrpcJsonClient_GetErrorString(session, code, nullptr, &size);
    if (error_code < 0) {
        return error_code;
    }
    unique_ptr<char> buffer(new char[size]);
    int32_t next_error_code = GrpcJsonClient_GetErrorString(session, code, buffer.get(), &size);
    if (next_error_code < 0) {
        return next_error_code;
    }
    if (message) {
        *message = buffer.get();
    }
    return error_code > 0 ? error_code : next_error_code;
}

void CheckErrorMessage(
    int32_t code, const string& message_body, const string& message_core, bool starts_with
) {
    const char* format = "Error Code: %d\nError Message: %s";
    int size = snprintf(nullptr, 0, format, code, message_core.c_str()) + 1;  // + null char
    unique_ptr<char> buffer(new char[size]);
    snprintf(buffer.get(), size, format, code, message_core.c_str());
    if (starts_with) {
        EXPECT_THAT(message_body, testing::StartsWith(buffer.get()));
    } else {
        EXPECT_EQ(message_body, buffer.get());
    }
}

void CheckGetErrorHelper(
    intptr_t session, int32_t expected_code, const string& message_core, bool starts_with
) {
    int32_t code = 0;
    string message;
    EXPECT_FALSE(GetErrorHelper(session, &code, &message));
    EXPECT_EQ(code, expected_code);
    EXPECT_NO_FATAL_FAILURE(CheckErrorMessage(code, message, message_core, starts_with));
}

void CheckGetErrorStringHelper(
    intptr_t session, int32_t code, const string& message_core, bool starts_with
) {
    string message;
    EXPECT_FALSE(GetErrorStringHelper(session, code, &message));
    EXPECT_NO_FATAL_FAILURE(CheckErrorMessage(code, message, message_core, starts_with));
}

}  // namespace grpc_json_client
}  // namespace ni
