#include "exceptions.h"

using std::string;

namespace ni
{
	ServiceDescriptorNotFoundException::ServiceDescriptorNotFoundException(const string& name)
	{
		_name = name;
	}

	MethodDescriptorNotFoundException::MethodDescriptorNotFoundException(const string& name)
	{
		_name = name;
	}
}
