
#include "testing_server.h"

#include <memory>
#include <string>
#include <vector>

#include <grpcpp/grpcpp.h>
#include <grpcpp/ext/proto_server_reflection_plugin.h>

using grpc::ChannelArguments;
using grpc::ServerBuilder;
using grpc::ServerBuilderOption;
using grpc::ServerBuilderPlugin;
using grpc::reflection::ProtoServerReflectionPlugin;
using std::string;
using std::vector;
using std::unique_ptr;

namespace ni {
namespace grpc_json_client {

class RemoveReflectionPlugin : public ServerBuilderOption {
    void UpdateArguments(ChannelArguments* args) override {}

    void UpdatePlugins(vector<unique_ptr<ServerBuilderPlugin>>* plugins) override {
        ProtoServerReflectionPlugin reflection_plugin;
        auto plugin = plugins->begin();
        while (plugin < plugins->end()) {
            if (plugin->get()->name() == reflection_plugin.name()) {
                plugin = plugins->erase(plugin);
            } else {
                plugin++;
            }
        }
    }
};

TestingServer::TestingServer(const string& address) :
    _address(address),
    _reflection_enabled(false) {}

void TestingServer::EnableReflection() {
    _reflection_enabled = true;
}

void TestingServer::Start() {
    ServerBuilder builder;
    builder.AddListeningPort(_address, grpc::InsecureServerCredentials());
    builder.RegisterService(&_service);
    if (!_reflection_enabled) {
        builder.SetOption(std::make_unique<RemoveReflectionPlugin>());
    }
    _server = builder.BuildAndStart();
}

void TestingServer::Wait() {
    _server->Wait();
}

void TestingServer::Stop() {
    _server->Shutdown();
    _server->Wait();
}

}  // namespace grpc_json_client
}  // namespace ni
