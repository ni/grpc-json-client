
#include <Windows.h>

#include "testing_server.h"

ni::grpc_json_client::TestingServer server("0.0.0.0:50051");

BOOL WINAPI event_handler(DWORD) {
    server.Stop();
    server.Wait();
    std::cout << "Server stopped." << std::endl;
    return TRUE;
}

int main() {
    server.EnableReflection();
    server.Start();
    SetConsoleCtrlHandler(event_handler, TRUE);
    std::cout << "Server started on port 50051." << std::endl;
    std::cout << "Press ctrl+c to stop." << std::endl;
    server.Wait();
}
