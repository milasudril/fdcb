//@	{"target":{"name":"fdcb.test"}}

#include "./fdcb.h"

#include <thread>

template<auto n>
struct dummy
{
};

size_t write(dummy<1>, std::span<std::byte const> buffer)
{
	write(STDERR_FILENO, ">>> ", 4);
	write(STDERR_FILENO, std::data(buffer), std::size(buffer));
	return std::size(buffer);
}

size_t write(dummy<2>, std::span<std::byte const> buffer)
{
	write(STDERR_FILENO, "<<< ", 4);
	write(STDERR_FILENO, std::data(buffer), std::size(buffer));
	return std::size(buffer);
}

int main()
{
	try
	{
		fdcb::context wrapper1{STDOUT_FILENO, dummy<1>{}};
		puts("Hello, World 1");
		// Must flush stdout, outerwise, there is nothing for the internal thread to read
		fflush(stdout);

		{
			fdcb::context wrapper2{STDOUT_FILENO, dummy<2>{}};
			puts("Hello, World 2");

			// Must flush stdout, outerwise, there is nothing for the internal thread to read
			fflush(stdout);
		}

		puts("Hello, World 1");
		// Must flush stdout, outerwise, there is nothing for the internal thread to read
		fflush(stdout);
	}
	catch(std::exception const& e)
	{
		fprintf(stderr, "%s\n", e.what());
	}
	puts("Hello, World (no longer redirected)");
}