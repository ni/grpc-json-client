
#pragma once

#include <memory>
#include <string>

#include "google/protobuf/descriptor.h"
#include "google/protobuf/descriptor_database.h"
#include "grpcpp/grpcpp.h"

namespace ni {
namespace grpc_json_client {

// Base class for json client implementations.
class JsonClientBase {
 protected:
    std::shared_ptr<grpc::Channel> channel;

 private:
    std::unique_ptr<google::protobuf::SimpleDescriptorDatabase> _descriptor_db;
    std::unique_ptr<google::protobuf::DescriptorPool> _descriptor_pool;

 public:
    JsonClientBase(
        const std::string& target, const std::shared_ptr<grpc::ChannelCredentials>& credentials);

    // Populate descriptor pool with file descriptors
    // for all services exposed by the reflection service on the host.
    void QueryReflectionService();

    // Search for a method in the descriptor database.
    const google::protobuf::MethodDescriptor* FindMethod(
        const std::string& service_name, const std::string& method_name) const;

 private:
    void ResetDescriptorPool();
};

}  // namespace grpc_json_client
}  // namespace ni
