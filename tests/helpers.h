#pragma once

#include <string>

int32_t MergeErrors(int32_t first, int32_t second);
int32_t BlockingCall(void* session_handle, const std::string& service, const std::string& method, const std::string& request, std::string& response);
