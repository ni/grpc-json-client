
#pragma once

#include <cstdio>
#include <memory>
#include <string>

namespace ni {
namespace grpc_json_client {

template <typename... Arguments>
std::string FormatString(const std::string& format, const Arguments&... args) {
    int size = snprintf(nullptr, 0, format.c_str(), args...) + 1;  // + null char
    std::unique_ptr<char> buffer(new char[size]);
    snprintf(buffer.get(), size, format.c_str(), args...);
    return buffer.get();
}

}  // namespace grpc_json_client
}  // namespace ni
