
#include "json_serializer.h"

#include <string>
#include <memory>

#include "google/protobuf/util/json_util.h"
#include "grpcpp/impl/codegen/proto_utils.h"

#include "exceptions.h"

using google::protobuf::Descriptor;
using google::protobuf::Message;
using google::protobuf::util::JsonOptions;
using grpc::ByteBuffer;
using grpc::ProtoBufferReader;
using grpc::ProtoBufferWriter;
using std::string;
using std::unique_ptr;

namespace ni  {
namespace grpc_json_client {

google::protobuf::DynamicMessageFactory JsonSerializer::factory;

ByteBuffer JsonSerializer::SerializeMessage(
    const Descriptor* message_type, const string& message_json
) {
    unique_ptr<Message> message = CreateMessage(message_type);
    google::protobuf::util::Status json_status = {
        google::protobuf::util::JsonStringToMessage(message_json, message.get())
    };
    if (!json_status.ok()) {
        string summary("Failed to create protobuf message from JSON string.\n\n");
        string details(json_status.message());
        throw SerializationException(summary + details);
    }
    ByteBuffer serialized_message;
    bool own_buffer = false;
    grpc::Status serialize_status = {
        grpc::GenericSerialize<ProtoBufferWriter, void>(*message, &serialized_message, &own_buffer)
    };
    if (!serialize_status.ok()) {
        string summary("Failed to serialize protobuf message.\n\n");
        throw SerializationException(summary + serialize_status.error_message());
    }
    return serialized_message;
}

string JsonSerializer::DeserializeMessage(
    const Descriptor* message_type, ByteBuffer* serialized_message
) {
    unique_ptr<Message> message = CreateMessage(message_type);
    grpc::Status deserialize_status = {
        grpc::GenericDeserialize<ProtoBufferReader, void>(serialized_message, message.get())
    };
    if (!deserialize_status.ok()) {
        string summary("Failed to deserialize protobuf message.\n\n");
        throw DeserializationException(summary + deserialize_status.error_message());
    }
    string response;
    JsonOptions json_options;
    json_options.always_print_primitive_fields = true;
    json_options.preserve_proto_field_names = true;
    google::protobuf::util::Status json_status = {
        google::protobuf::util::MessageToJsonString(*message, &response, json_options)
    };
    if (!json_status.ok()) {
        string summary("Failed to create JSON string from protobuf message.\n\n");
        string details(json_status.message());
        throw DeserializationException(summary + details);
    }
    return response;
}

unique_ptr<Message> JsonSerializer::CreateMessage(const Descriptor* message_type) {
    const Message* message_prototype = factory.GetPrototype(message_type);
    return unique_ptr<Message>(message_prototype->New());
}

}  // namespace grpc_json_client
}  // namespace ni
