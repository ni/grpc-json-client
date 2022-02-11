#pragma once

#include <stdexcept>

namespace ni
{
	class DynamicClientException : std::exception
	{
	public:
		DynamicClientException() {}
	};

	class ServiceUnavailableException : DynamicClientException
	{
	public:
		ServiceUnavailableException() = default;
	};

	class ServiceDescriptorNotFoundException : DynamicClientException
	{
	public:
		ServiceDescriptorNotFoundException(const std::string& name);

	private:
		std::string _name;
	};

	class MethodDescriptorNotFoundException : DynamicClientException
	{
	public:
		MethodDescriptorNotFoundException(const std::string& name);

	private:
		std::string _name;
	};
}
