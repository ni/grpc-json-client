#include "dynamic_client.h"

#include <google/protobuf/text_format.h>
#include <google/protobuf/util/json_util.h>
#include <grpcpp/grpcpp.h>

#include "exceptions.h"
#include "reflection.pb.h"
#include "reflection.grpc.pb.h"

using google::protobuf::Descriptor;
using google::protobuf::DescriptorPool;
using google::protobuf::DynamicMessageFactory;
using google::protobuf::FileDescriptorProto;
using google::protobuf::Message;
using google::protobuf::MethodDescriptor;
using google::protobuf::RepeatedPtrField;
using google::protobuf::ServiceDescriptor;
using google::protobuf::TextFormat;
using google::protobuf::util::JsonOptions;
using grpc::ByteBuffer;
using grpc::ChannelCredentials;
using grpc::ClientContext;
using grpc::ClientReaderWriter;
using grpc::ProtoBufferReader;
using grpc::ProtoBufferWriter;
using grpc::reflection::v1alpha::ServerReflection;
using grpc::reflection::v1alpha::ServerReflectionRequest;
using grpc::reflection::v1alpha::ServerReflectionResponse;
using grpc::reflection::v1alpha::ServiceResponse;
using std::shared_ptr;
using std::string;
using std::unique_ptr;

namespace ni
{
	DynamicClient::DynamicClient(const string& target)
	{
		shared_ptr<ChannelCredentials> insecure_credentials = grpc::InsecureChannelCredentials();
		channel = CreateChannel(target, insecure_credentials);
		ServerReflection::Stub reflection_stub(channel);

		// request server to list services
		ClientContext context;
		unique_ptr<ClientReaderWriter<ServerReflectionRequest, ServerReflectionResponse>> stream = reflection_stub.ServerReflectionInfo(&context);
		ServerReflectionRequest request;
		request.set_list_services("");  // content will not be checked
		stream->Write(request);

		// read services
		ServerReflectionResponse response;
		stream->Read(&response);
		RepeatedPtrField<ServiceResponse> services = response.list_services_response().service();

		// request file descriptors for services
		for (ServiceResponse service : services)
		{
			request.set_file_containing_symbol(service.name());
			stream->Write(request);
			stream->Read(&response);
			auto serialized_file_descriptors = response.file_descriptor_response().file_descriptor_proto();
			for (string serialized_file_descriptor : serialized_file_descriptors)
			{
				FileDescriptorProto file_descriptor_proto;
				file_descriptor_proto.ParseFromString(serialized_file_descriptor);
				if (!_reflection_db.FindFileByName(file_descriptor_proto.name(), &file_descriptor_proto))
				{
					_reflection_db.Add(file_descriptor_proto);
				}
			}
		}

		// close stream
		stream->WritesDone();
		grpc::Status status = stream->Finish();
		if (!status.ok())
		{
			// todo
		}

		_descriptor_pool = std::make_unique<DescriptorPool>(&_reflection_db);
		_message_factory = std::make_unique<DynamicMessageFactory>(_descriptor_pool.get());
	}

	const MethodDescriptor* DynamicClient::FindMethod(const string& service_name, const string& method_name)
	{
		const ServiceDescriptor* service_descriptor = _descriptor_pool->FindServiceByName(service_name);
		if (service_descriptor == nullptr)
		{
			throw ServiceNotFoundException(service_name);
		}
		const MethodDescriptor* method_descriptor = service_descriptor->FindMethodByName(method_name);
		if (method_descriptor == nullptr)
		{
			throw MethodNotFoundException(method_name);
		}
		return method_descriptor;
	}

	ByteBuffer DynamicClient::SerializeMessage(const Descriptor* message_type, const string& message_json)
	{
		unique_ptr<Message> message = CreateMessage(message_type);
		google::protobuf::util::Status json_status = google::protobuf::util::JsonStringToMessage(message_json, message.get());
		if (!json_status.ok())
		{
			// todo
		}
		ByteBuffer serialized_message;
		bool own_buffer = false;
		grpc::Status serialize_status = grpc::GenericSerialize<ProtoBufferWriter, void>(*message, &serialized_message, &own_buffer);
		if (!serialize_status.ok())
		{
			// todo
		}
		return serialized_message;
	}

	string DynamicClient::DeserializeMessage(const Descriptor* message_type, ByteBuffer& serialized_message)
	{
		unique_ptr<Message> message = CreateMessage(message_type);
		grpc::Status deserialize_status = grpc::GenericDeserialize<ProtoBufferReader, void>(&serialized_message, message.get());
		if (!deserialize_status.ok())
		{
			// todo
		}
		string response;
		JsonOptions json_options;
		json_options.always_print_primitive_fields = true;
		json_options.preserve_proto_field_names = true;
		google::protobuf::util::Status json_status = google::protobuf::util::MessageToJsonString(*message, &response, json_options);
		if (!json_status.ok())
		{
			// todo
		}
		return response;
	}

	unique_ptr<Message> DynamicClient::CreateMessage(const Descriptor* message_type)
	{
		const Message* message_prototype = _message_factory->GetPrototype(message_type);
		return unique_ptr<Message>(message_prototype->New());
	}
}
