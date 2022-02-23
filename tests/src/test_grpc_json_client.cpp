#include <iostream>
#include <memory>
#include <string>
#include <stdint.h>

#include <grpcpp/ext/proto_server_reflection_plugin.h>
#include <grpcpp/grpcpp.h>
#include <gtest/gtest.h>
#include <nlohmann/json.hpp>

#include "grpc_json_client.h"
#include "testing_server.h"

using grpc::Server;
using grpc::ServerBuilder;
using nlohmann::json;
using std::string;
using std::unique_ptr;

namespace ni
{
    namespace grpc_json_client
    {
        const std::string host = "localhost:50051";

        class GrpcJsonClientTest : public testing::Test
        {
        protected:
            void* session = nullptr;
            static unique_ptr<TestingServiceImpl> service;
            static unique_ptr<Server> server;

            static void SetUpTestSuite()
            {
                grpc::reflection::InitProtoReflectionServerBuilderPlugin();
                string address("0.0.0.0:50051");
                ServerBuilder builder;
                builder.AddListeningPort(address, grpc::InsecureServerCredentials());
                builder.RegisterService(service.get());
                server = builder.BuildAndStart();
            }

            static void TearDownTestSuite()
            {
                server->Shutdown();
                server->Wait();
            }

            void SetUp() override
            {
                ASSERT_EQ(GrpcJsonClient_InitInsecure(host.c_str(), &session), 0);
            }

            void TearDown() override
            {
                ASSERT_EQ(GrpcJsonClient_Close(session), 0);
            }
        };

        unique_ptr<TestingServiceImpl> GrpcJsonClientTest::service = std::make_unique<TestingServiceImpl>();
        unique_ptr<Server> GrpcJsonClientTest::server;
        
        TEST_F(GrpcJsonClientTest, SynchronousCallsSucceed)
        {
            for (string string_field : { "first", "second" })
            {
                json request = { {"string_field", string_field} };
                void* tag = nullptr;
                ASSERT_EQ(GrpcJsonClient_StartAsyncCall(session, "ni.grpc_json_client.TestingService", "UnaryUnaryEcho", request.dump().c_str(), &tag), 0);
                size_t size = 0;
                ASSERT_EQ(GrpcJsonClient_FinishAsyncCall(session, tag, -1, nullptr, &size), 0);
                char* buffer = new char[size];
                ASSERT_EQ(GrpcJsonClient_FinishAsyncCall(session, tag, -1, buffer, &size), 0);
                json response = json::parse(buffer);
                delete[] buffer;
                ASSERT_EQ(response["string_field"], string_field);
            }
        }

        TEST_F(GrpcJsonClientTest, AsynchronousCallsSucceed)
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
                ASSERT_EQ(GrpcJsonClient_FinishAsyncCall(session, tags[i], 1000, nullptr, &size), 0);
                char* buffer = new char[size];
                ASSERT_EQ(GrpcJsonClient_FinishAsyncCall(session, tags[i], 1000, buffer, &size), 0);
                json response = json::parse(buffer);
                delete[] buffer;
                ASSERT_EQ(response["string_field"], string_fields[i]);
            }
        }
    }
}
