#include <stdint.h>

#include <gtest/gtest.h>
#include <nlohmann/json.hpp>

#include "grpc_json_client.h"

using nlohmann::json;
using std::unique_ptr;

namespace ni
{
    namespace grpc_json_client
    {
        const std::string host = "localhost:31763";

        class GrpcJsonClientTest : public testing::Test
        {
        protected:
            void* session = nullptr;

            GrpcJsonClientTest() = default;

            void SetUp() override
            {
                ASSERT_EQ(InitInsecure(host.c_str(), &session), 0);
            }

            void TearDown() override
            {
                Close(session);
            }
        };

        /*
        TEST_F(GrpcJsonClientTest, WriteSucceedsOnValidWrite)
        {
            json request = { {"session_name", "gtest"}, {"resource_name", "VST2_01"} };
            ASSERT_EQ(Write(session, "nirfsa_grpc.NiRFSA", "Init", request.dump().c_str()), 0);
            size_t size = 0;
            ASSERT_EQ(Read(session, nullptr, &size), 0);
            unique_ptr<char> buffer(new char[size]);
            ASSERT_EQ(Read(session, buffer.get(), &size), 0);
            json response = json::parse(buffer.get());
            ASSERT_EQ(response["status"], 0);
            ASSERT_TRUE(response["vi"].contains("id"));
            request.clear();
            request["vi"] = response["vi"];
            ASSERT_EQ(Write(session, "nirfsa_grpc.NiRFSA", "Close", request.dump().c_str()), 0);
        }
        */
        
        TEST_F(GrpcJsonClientTest, WriteSucceedsOnValidWrite)
        {
            json request = { {"session_name", "gtest"}, {"resource_name", "VST2_01"} };
            ASSERT_EQ(Write(session, "nirfsa_grpc.NiRFSA", "Init", request.dump().c_str()), 0);

            int timeout = 10000;
            size_t size = 0;
            ASSERT_EQ(Read(session, timeout, nullptr, &size), 0);
            unique_ptr<char> buffer(new char[size]);
            ASSERT_EQ(Read(session, timeout, buffer.get(), &size), 0);
            json response = json::parse(buffer.get());
            ASSERT_EQ(response["status"], 0);
            json vi = response["vi"];

            request.clear();
            request["vi"] = vi;
            request["attribute_id"] = "NIRFSA_ATTRIBUTE_REFERENCE_LEVEL";
            request["value_raw"] = 40.0;
            ASSERT_EQ(Write(session, "nirfsa_grpc.NiRFSA", "SetAttributeViReal64", request.dump().c_str()), 0);
            
            request.clear();
            request["vi"] = vi;
            ASSERT_EQ(Write(session, "nirfsa_grpc.NiRFSA", "Commit", request.dump().c_str()), 0);

            ASSERT_EQ(Read(session, timeout, nullptr, &size), 0);
            buffer = unique_ptr<char>(new char[size]);
            ASSERT_EQ(Read(session, timeout, buffer.get(), &size), 0);
            response = json::parse(buffer.get());
            ASSERT_EQ(response["status"], 0);

            ASSERT_EQ(Read(session, timeout, nullptr, &size), 0);
            buffer = unique_ptr<char>(new char[size]);
            ASSERT_EQ(Read(session, timeout, buffer.get(), &size), 0);
            response = json::parse(buffer.get());
            ASSERT_EQ(response["status"], 0);
        }
    }
}
