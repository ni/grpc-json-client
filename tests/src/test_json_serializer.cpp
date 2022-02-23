#include <gtest/gtest.h>

#include "exceptions.h"
#include "json_serializer.h"
#include "reflection.pb.h"

using google::protobuf::Descriptor;
using grpc::ByteBuffer;
using grpc::reflection::v1alpha::FileDescriptorResponse;
using grpc::reflection::v1alpha::ServerReflectionRequest;
using std::string;

namespace ni
{
    namespace grpc_json_client
    {
        TEST(JsonSerializerTest, SerializesJsonObject)
        {
            const Descriptor* message_type = ServerReflectionRequest::GetDescriptor();
            string valid_json("{\"host\":\"localhost\",\"file_by_filename\":\"session.proto\"}");
            ASSERT_NO_THROW(
                JsonSerializer::SerializeMessage(message_type, valid_json)
            );
        }

        TEST(JsonSerializerTest, SerializesEmptyJsonObject)
        {
            const Descriptor* message_type = ServerReflectionRequest::GetDescriptor();
            ASSERT_NO_THROW(
                JsonSerializer::SerializeMessage(message_type, "{}")
            );
        }

        TEST(JsonSerializerTest, SerializeFailsOnEmptyString)
        {
            const Descriptor* message_type = ServerReflectionRequest::GetDescriptor();
            ASSERT_THROW(
                JsonSerializer::SerializeMessage(message_type, ""),
                SerializationException
            );
        }

        TEST(JsonSerializerTest, SerializeFailsWithMalformedString)
        {
            const Descriptor* message_type = ServerReflectionRequest::GetDescriptor();
            string missing_last_curly_brace("{\"host\":\"localhost\",\"file_by_filename\":\"session.proto\"");
            ASSERT_THROW(
                JsonSerializer::SerializeMessage(message_type, missing_last_curly_brace),
                SerializationException
            );
        }

        TEST(JsonSerializerTest, SerializeFailsWithInvalidProperty)
        {
            const Descriptor* message_type = ServerReflectionRequest::GetDescriptor();
            string has_extra_name_field("{\"host\":\"localhost\",\"file_by_filename\":\"session.proto\",\"name\":\"ni\"}");
            ASSERT_THROW(
                JsonSerializer::SerializeMessage(message_type, has_extra_name_field),
                SerializationException
            );
        }

        TEST(JsonSerializerTest, SerializeFailsOnInvalidMessageType)
        {
            const Descriptor* message_type = FileDescriptorResponse::GetDescriptor();
            string request("{\"host\":\"localhost\",\"file_by_filename\":\"session.proto\"}");
            ASSERT_THROW(
                JsonSerializer::SerializeMessage(message_type, request),
                SerializationException
            );
        }

        TEST(JsonSerializerTest, DeserializeSucceedsWithValidByteBuffer)
        {
            const Descriptor* message_type = ServerReflectionRequest::GetDescriptor();
            string request("{\"host\":\"localhost\",\"file_by_filename\":\"session.proto\"}");
            ByteBuffer buffer = JsonSerializer::SerializeMessage(message_type, request);
            string response = JsonSerializer::DeserializeMessage(message_type, buffer);
            ASSERT_EQ(request, response);
        }

        TEST(JsonSerializerTest, DeserializeFailsWithEmptyByteBuffer)
        {
            const Descriptor* response_type = FileDescriptorResponse::GetDescriptor();
            ByteBuffer empty_buffer;
            ASSERT_THROW
            (
                JsonSerializer::DeserializeMessage(response_type, empty_buffer),
                DeserializationException
            );
        }
    }
}
