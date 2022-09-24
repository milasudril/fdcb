//@	{"target":{"name":"fdcb.test"}}

#include "./fdcb.h"

struct dummy
{
};

ssize_t write(dummy, std::span<std::byte const>)
{
	return -1;
}

int main()
{
	try
	{
		fdcb::context wrapper{STDOUT_FILENO, dummy{}};
	}
	catch(std::exception const& e)
	{
		fprintf(stderr, "%s\n", e.what());
	}
	puts("Hello, World");
}