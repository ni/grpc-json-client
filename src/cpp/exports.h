#pragma once

#include "dynamic_client.h"

extern "C" __declspec(dllexport) int Init(const char* target, ni::DynamicClient** const session_handle);
extern "C" __declspec(dllexport) int Query(ni::DynamicClient* const session_handle, const char* service, const char* method, const char* request, string** const response_handle);
extern "C" __declspec(dllexport) int ReadResponse(string* const response_handle, char* response, size_t* const size);
extern "C" __declspec(dllexport) int Close(ni::DynamicClient* const session_handle);
