
#pragma once

#include <memory>
#include <string>

#include "google/protobuf/descriptor.h"
#include "google/protobuf/descriptor_database.h"
#include "grpcpp/grpcpp.h"
#include "reflection.pb.h"

namespace ni {
namespace grpc_json_client {

// Base class for json client implementations.
class JsonClientBase {
 protected:
    std::shared_ptr<grpc::Channel> channel;

 private:
    std::unique_ptr<google::protobuf::SimpleDescriptorDatabase> _database;
    std::unique_ptr<google::protobuf::DescriptorPool> _pool;

 public:
    JsonClientBase(
        const std::string& target, const std::shared_ptr<grpc::ChannelCredentials>& credentials);

    // Resets the descriptor database to it's default state.
    void ResetDescriptorDatabase();

    // Populate descriptor database with file descriptors
    // for all services exposed by the reflection service on the host.
    void JsonClientBase::FillDescriptorDatabase();

    // Search for a method in the descriptor database.
    const google::protobuf::MethodDescriptor* FindMethod(
        const std::string& service_name, const std::string& method_name);

 private:
    void QueryReflectionService(
        const grpc::reflection::v1alpha::ServerReflectionRequest& request,
        grpc::reflection::v1alpha::ServerReflectionResponse* response);

    void FetchFileDescriptors(const std::string& symbol);
};

}  // namespace grpc_json_client
}  // namespace ni
