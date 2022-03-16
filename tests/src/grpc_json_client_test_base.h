#pragma once

#include <cstdint>
#include <memory>

#include <gtest/gtest.h>

#include "testing_server.h"

namespace ni {
namespace grpc_json_client {

class GrpcJsonClientTestBase : public testing::Test {
protected:
	static const char* address;
    static std::unique_ptr<TestingServer> server;
    static const char* service;
    static const char* echo;

    intptr_t session;

    GrpcJsonClientTestBase();
    void SetUp() override;
    void TearDown() override;
};

}  // namespace grpc_json_client
}  // namespace ni
