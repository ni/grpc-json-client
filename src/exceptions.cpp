#include "exceptions.h"

using namespace std;
using namespace ni;

ServiceNotFoundException::ServiceNotFoundException(const string& name)
{
	_name = name;
}

MethodNotFoundException::MethodNotFoundException(const string& name)
{
	_name = name;
}
