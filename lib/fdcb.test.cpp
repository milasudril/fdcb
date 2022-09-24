//@	{"target":{"name":"fdcb.test"}}

#include "./fdcb.h"

#include <thread>

struct dummy
{
};

size_t write(dummy, std::span<std::byte const> buffer)
{
	write(STDERR_FILENO, ">>> ", 4);
	write(STDERR_FILENO, std::data(buffer), std::size(buffer));
	return std::size(buffer);
}

int main()
{
	try
	{
		fdcb::context wrapper{STDOUT_FILENO, dummy{}};
		puts("Hello, World");

		// Must flush stdout, outerwise, there is nothing for the internal thread to read
		fflush(stdout);
	}
	catch(std::exception const& e)
	{
		fprintf(stderr, "%s\n", e.what());
	}
	puts("Hello, World (no longer redirected)");
}