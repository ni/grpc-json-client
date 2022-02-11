#pragma once

#include <stdexcept>

namespace ni
{
	class DynamicClientException : public std::exception
	{
	public:
		explicit DynamicClientException(const std::string& _Message) : std::exception(_Message.c_str()) {}
	};
	
	class ServiceUnavailableException : public DynamicClientException
	{
		using DynamicClientException::DynamicClientException;
	};

	class ServiceDescriptorNotFoundException : public DynamicClientException
	{
	public:
		ServiceDescriptorNotFoundException(const std::string& name) : DynamicClientException(name) {}
	};

	class MethodDescriptorNotFoundException :public  DynamicClientException
	{
	public:
		MethodDescriptorNotFoundException(const std::string& name) : DynamicClientException(name) {}
	};
	
	class SerializationException : public DynamicClientException
	{
		using DynamicClientException::DynamicClientException;
	};
	
	class DeserializationException : public DynamicClientException
	{
		using DynamicClientException::DynamicClientException;
	};
}
