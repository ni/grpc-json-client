#include <stdint.h>

#include <gtest/gtest.h>

#include "grpc_json_client.h"

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
                InitInsecure(host.c_str(), &session);
            }
        };

        TEST_F(GrpcJsonClientTest, WriteSucceedsOnValidWrite)
        {
            const char* valid_json = "{\"session_name\":\"gtest\",\"resource_name\":\"VST2_01\"}";
            int32_t error_code = Write(session, "nirfsa_grpc.NiRFSA", "Init", valid_json);
            ASSERT_EQ(error_code, 0);
            size_t size;
            error_code = Read(session, nullptr, &size);
            ASSERT_EQ(error_code, 0);
            unique_ptr<char> buffer(new char[size]);
            error_code = Read(session, buffer.get(), &size);
            ASSERT_EQ(error_code, 0);
            const char* expected_response = "{\"status\":0,\"vi\":{}}";
            ASSERT_STREQ(buffer.get(), expected_response);
        }
    }
}
