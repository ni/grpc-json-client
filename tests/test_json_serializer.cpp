#include <gtest/gtest.h>

#include "json_serializer.h"
#include "reflection.pb.h"

using grpc::reflection::v1alpha::ServerReflectionRequest;

namespace ni
{
	TEST(JsonSerializerTest, SerializeMessageSerializesValidString)
	{
		ServerReflectionRequest request;
		JsonSerializer::SerializeMessage(ServerReflectionRequest::GetDescriptor(), { "{\"host\": \"localhost\", \"file_by_filename\": \"session.proto\"}" });
	}
}