#pragma once

#include <google/protobuf/descriptor.h>
#include <google/protobuf/dynamic_message.h>
#include <grpcpp/grpcpp.h>

namespace ni
{
    namespace json_client
    {
        // Contains utilities for serializing messages to and from JSON.
        class JsonSerializer
        {
        private:
            static google::protobuf::DynamicMessageFactory factory;
            
        public:
            // Deserialize a protobuf message from a JSON string then serialize it into a byte buffer.
            static grpc::ByteBuffer SerializeMessage(const google::protobuf::Descriptor* message_type, const std::string& message_json);

            // Deserialize a message from a byte buffer then serialize it into a JSON string.
            static std::string DeserializeMessage(const google::protobuf::Descriptor* message_type, grpc::ByteBuffer& serialized_message);

        private:
            // Create a message instance from a message type.
            static std::unique_ptr<google::protobuf::Message> CreateMessage(const google::protobuf::Descriptor* message_type);
        };
    }
}
