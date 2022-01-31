#pragma once

#include <stdexcept>

namespace ni
{
	class DynamicClientException : std::exception
	{
	public:
		DynamicClientException() {}
	};

	class ServiceNotFoundException : DynamicClientException
	{
	public:
		ServiceNotFoundException(const std::string& name);

	private:
		std::string _name;
	};

	class MethodNotFoundException : DynamicClientException
	{
	public:
		MethodNotFoundException(const std::string& name);

	private:
		std::string _name;
	};
}
