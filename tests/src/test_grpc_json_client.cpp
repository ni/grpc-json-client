#include <memory>
#include <string>

#include <gtest/gtest.h>
#include <nlohmann/json.hpp>

#include "error_code.h"
#include "grpc_json_client.h"
#include "testing_server.h"

using nlohmann::json;
using std::string;
using std::unique_ptr;

namespace ni
{
    namespace grpc_json_client
    {
        class GrpcJsonClientTest : public testing::Test
        {
        protected:
            static unique_ptr<TestingServer> server;
            void* session = nullptr;

            static void SetUpTestSuite()
            {
                string address("0.0.0.0:50051");
                server = std::make_unique<TestingServer>(address);
                server->StartInsecure();
            }

            static void TearDownTestSuite()
            {
                server->Stop();
            }

            void SetUp() override
            {
                ASSERT_EQ(GrpcJsonClient_InitInsecure("localhost:50051", &session), 0);
            }

            void TearDown() override
            {
                ASSERT_EQ(GrpcJsonClient_Close(session), 0);
            }
        };

        unique_ptr<TestingServer> GrpcJsonClientTest::server;
        
        TEST_F(GrpcJsonClientTest, SynchronousCallsWithTimeoutsSucceed)
        {
            for (string string_field : { "first", "second" })
            {
                json request = { {"string_field", string_field} };
                void* tag = nullptr;
                ASSERT_EQ(GrpcJsonClient_StartAsyncCall(session, "ni.grpc_json_client.TestingService", "UnaryUnaryEcho", request.dump().c_str(), &tag), 0);
                size_t size = 0;
                ASSERT_EQ(GrpcJsonClient_FinishAsyncCall(session, tag, 100, nullptr, &size), 0);
                unique_ptr<char> buffer(new char[size]);
                ASSERT_EQ(GrpcJsonClient_FinishAsyncCall(session, tag, 100, buffer.get(), &size), 0);
                json response = json::parse(buffer.get());
                ASSERT_EQ(response["string_field"], string_field);
            }
        }

        TEST_F(GrpcJsonClientTest, SynchronousCallsWithoutTimeoutsSucceed)
        {
            for (string string_field : { "first", "second" })
            {
                json request = { {"delay", 100}, {"string_field", string_field} };
                void* tag = nullptr;
                ASSERT_EQ(GrpcJsonClient_StartAsyncCall(session, "ni.grpc_json_client.TestingService", "UnaryUnaryEcho", request.dump().c_str(), &tag), 0);
                size_t size = 0;
                ASSERT_EQ(GrpcJsonClient_FinishAsyncCall(session, tag, -1, nullptr, &size), 0);
                unique_ptr<char> buffer(new char[size]);
                ASSERT_EQ(GrpcJsonClient_FinishAsyncCall(session, tag, -1, buffer.get(), &size), 0);
                json response = json::parse(buffer.get());
                ASSERT_EQ(response["string_field"], string_field);
            }
        }

        TEST_F(GrpcJsonClientTest, AsynchronousCallsWithTimeoutsSucceed)
        {
            void* tags[] = { nullptr, nullptr };
            string string_fields[] = { "first", "second" };

            for (size_t i = 0; i < 2; i++)
            {
                json request = { {"string_field", string_fields[i]} };
                ASSERT_EQ(GrpcJsonClient_StartAsyncCall(session, "ni.grpc_json_client.TestingService", "UnaryUnaryEcho", request.dump().c_str(), &tags[i]), 0);
            }

            for (size_t i = 0; i < 2; i++)
            {
                size_t size = 0;
                ASSERT_EQ(GrpcJsonClient_FinishAsyncCall(session, tags[i], 100, nullptr, &size), 0);
                unique_ptr<char> buffer(new char[size]);
                ASSERT_EQ(GrpcJsonClient_FinishAsyncCall(session, tags[i], 100, buffer.get(), &size), 0);
                json response = json::parse(buffer.get());
                ASSERT_EQ(response["string_field"], string_fields[i]);
            }
        }

        TEST_F(GrpcJsonClientTest, AsynchronousCallsWithoutTimeoutsSucceed)
        {
            void* tags[] = { nullptr, nullptr };
            string string_fields[] = { "first", "second" };

            for (size_t i = 0; i < 2; i++)
            {
                json request = { {"delay", 100}, {"string_field", string_fields[i]} };
                ASSERT_EQ(GrpcJsonClient_StartAsyncCall(session, "ni.grpc_json_client.TestingService", "UnaryUnaryEcho", request.dump().c_str(), &tags[i]), 0);
            }

            for (size_t i = 0; i < 2; i++)
            {
                size_t size = 0;
                ASSERT_EQ(GrpcJsonClient_FinishAsyncCall(session, tags[i], -1, nullptr, &size), 0);
                unique_ptr<char> buffer(new char[size]);
                ASSERT_EQ(GrpcJsonClient_FinishAsyncCall(session, tags[i], -1, buffer.get(), &size), 0);
                json response = json::parse(buffer.get());
                ASSERT_EQ(response["string_field"], string_fields[i]);
            }
        }

        TEST_F(GrpcJsonClientTest, StartAsyncCallToUndefinedServiceFailsWithServiceNotFoundError)
        {
            void* tag = nullptr;
            int32_t error_code = GrpcJsonClient_StartAsyncCall(session, "UndefinedService", "UndefinedMethod", "{}", &tag);
            ASSERT_EQ(error_code, static_cast<int32_t>(ErrorCode::kServiceNotFoundError));
        }

        TEST_F(GrpcJsonClientTest, StartAsyncCallToUndefinedMethodFailsWithMethodNotFoundError)
        {
            void* tag = nullptr;
            int32_t error_code = GrpcJsonClient_StartAsyncCall(session, "ni.grpc_json_client.TestingService", "UndefinedMethod", "{}", &tag);
            ASSERT_EQ(error_code, static_cast<int32_t>(ErrorCode::kMethodNotFoundError));
        }

        TEST_F(GrpcJsonClientTest, StartAsyncCallWithMalformedMessageFailsWithSerializationError)
        {
            void* tag = nullptr;
            int32_t error_code = GrpcJsonClient_StartAsyncCall(session, "ni.grpc_json_client.TestingService", "UnaryUnaryEcho", "", &tag);
            ASSERT_EQ(error_code, static_cast<int32_t>(ErrorCode::kSerializationError));
        }

        TEST_F(GrpcJsonClientTest, FinishAsyncCallWithInvalidTagFailsWithInvalidTagError)
        {
            void* tag = nullptr;
            ASSERT_EQ(GrpcJsonClient_StartAsyncCall(session, "ni.grpc_json_client.TestingService", "UnaryUnaryEcho", "{}", &tag), 0);

            size_t size = 0;
            tag = (int*)tag + 1;
            int32_t error_code = GrpcJsonClient_FinishAsyncCall(session, tag, 100, nullptr, &size);
            ASSERT_EQ(error_code, static_cast<int32_t>(ErrorCode::kInvalidTagError));
        }

        TEST_F(GrpcJsonClientTest, CallWithShortTimeoutFailsWithTimeoutError)
        {
            json request = { {"delay", 100} };
            void* tag = nullptr;
            ASSERT_EQ(GrpcJsonClient_StartAsyncCall(session, "ni.grpc_json_client.TestingService", "UnaryUnaryEcho", request.dump().c_str(), &tag), 0);

            size_t size = 0;
            int32_t error_code = GrpcJsonClient_FinishAsyncCall(session, tag, 10, nullptr, &size);
            ASSERT_EQ(error_code, static_cast<int32_t>(ErrorCode::kTimeoutError));
        }

        TEST_F(GrpcJsonClientTest, FinishAsyncCallWithSmallBufferFailsWithBufferSizeOutOfRangeError)
        {
            void* tag = nullptr;
            ASSERT_EQ(GrpcJsonClient_StartAsyncCall(session, "ni.grpc_json_client.TestingService", "UnaryUnaryEcho", "{}", &tag), 0);

            size_t size = 0;
            char* buffer = (char*)1;
            int32_t error_code = GrpcJsonClient_FinishAsyncCall(session, tag, 100, buffer, &size);
            ASSERT_EQ(error_code, static_cast<int32_t>(ErrorCode::kBufferSizeOutOfRangeError));
        }

        TEST_F(GrpcJsonClientTest, GetErrorWithSessionSucceeds)
        {
            void* tag = nullptr;
            int32_t expected_error_code = GrpcJsonClient_StartAsyncCall(session, "UndefinedService", "UndefinedMethod", "{}", &tag);

            int32_t queried_error_code = 0;
            size_t size = 0;
            ASSERT_EQ(GrpcJsonClient_GetError(session, &queried_error_code, nullptr, &size), 0);
            unique_ptr<char> buffer(new char[size]);
            ASSERT_EQ(GrpcJsonClient_GetError(session, &queried_error_code, buffer.get(), &size), 0);

            ASSERT_EQ(queried_error_code, expected_error_code);  // returns most recent code
        }

        TEST_F(GrpcJsonClientTest, GetErrorWithoutSessionSucceeds)
        {
            void* tag = nullptr;
            int32_t error_code = GrpcJsonClient_StartAsyncCall(session, "UndefinedService", "UndefinedMethod", "{}", &tag);

            size_t size = 0;
            ASSERT_EQ(GrpcJsonClient_GetError(nullptr, &error_code, nullptr, &size), 0);
            unique_ptr<char> buffer(new char[size]);
            ASSERT_EQ(GrpcJsonClient_GetError(nullptr, &error_code, buffer.get(), &size), 0);

            string expected_description = GetErrorDescription(static_cast<ErrorCode>(error_code));
            ASSERT_STREQ(buffer.get(), expected_description.c_str());
        }

        TEST_F(GrpcJsonClientTest, GetErrorWithSmallBufferSucceedsWithBufferSizeOutOfRangeWarning)
        {
            void* tag = nullptr;
            GrpcJsonClient_StartAsyncCall(session, "UndefinedService", "UndefinedMethod", "{}", &tag);

            int32_t queried_error_code = 0;
            char buffer = 0;
            size_t size = 1;
            int32_t error_code = GrpcJsonClient_GetError(session, &queried_error_code, &buffer, &size);

            ASSERT_EQ(error_code, static_cast<int32_t>(ErrorCode::kBufferSizeOutOfRangeWarning));
        }
    }
}
