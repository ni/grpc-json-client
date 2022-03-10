
#include "grpcpp/ext/proto_server_reflection_plugin.h"
#include "gtest/gtest.h"

int main(int argc, char** argv) {
    grpc::reflection::InitProtoReflectionServerBuilderPlugin();
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
