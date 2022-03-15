
#include "json_serializer.h"

#include <google/protobuf/stubs/status.h>
#include <google/protobuf/util/json_util.h>
#include <grpcpp/impl/codegen/proto_utils.h>

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

void JsonSerializer::JsonStringToMessage(const string& json, Message* message) {
    google::protobuf::util::Status status = {
        google::protobuf::util::JsonStringToMessage(json, message)
    };
    if (!status.ok()) {
        string summary("Failed to create protobuf message from JSON string.");
        string details(status.message());
        throw SerializationException(summary, details);
    }
}

ByteBuffer JsonSerializer::SerializeMessage(
    const Descriptor* message_type, const string& message_json
) {
    unique_ptr<Message> message = CreateMessage(message_type);
    JsonStringToMessage(message_json, message.get());
    ByteBuffer serialized_message;
    bool own_buffer = false;
    grpc::Status status = {
        grpc::GenericSerialize<ProtoBufferWriter, void>(*message, &serialized_message, &own_buffer)
    };
    if (!status.ok()) {
        string summary("Failed to serialize protobuf message.");
        throw SerializationException(summary, status.error_message());
    }
    return serialized_message;
}

string JsonSerializer::MessageToJsonString(const Message& message) {
    string json;
    JsonOptions options;
    options.always_print_primitive_fields = true;
    options.preserve_proto_field_names = true;
    google::protobuf::util::Status status = {
        google::protobuf::util::MessageToJsonString(message, &json, options)
    };
    if (!status.ok()) {
        string summary("Failed to create JSON string from protobuf message.");
        string details(status.message());
        throw DeserializationException(summary, details);
    }
    return json;
}

string JsonSerializer::DeserializeMessage(
    const Descriptor* message_type, ByteBuffer* serialized_message
) {
    unique_ptr<Message> message = CreateMessage(message_type);
    grpc::Status deserialize_status = {
        grpc::GenericDeserialize<ProtoBufferReader, void>(serialized_message, message.get())
    };
    if (!deserialize_status.ok()) {
        string summary("Failed to deserialize protobuf message.");
        throw DeserializationException(summary, deserialize_status.error_message());
    }
    return MessageToJsonString(*message);
}

unique_ptr<Message> JsonSerializer::CreateMessage(const Descriptor* message_type) {
    const Message* message_prototype = factory.GetPrototype(message_type);
    return unique_ptr<Message>(message_prototype->New());
}

}  // namespace grpc_json_client
}  // namespace ni
