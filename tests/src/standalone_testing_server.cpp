
#include <Windows.h>

#include "testing_server.h"

using ni::grpc_json_client::TestingServer;

TestingServer server("0.0.0.0:50051");

void signal_handler(int signal) {
    server.Stop();
    std::cout << "Server stopped." << std::endl;
}

int main() {
    server.EnableReflection();
    server.Start();
    std::cout << "Server started on port 50051." << std::endl;
    server.Wait();
}
