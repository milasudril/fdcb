//@	{"target":{"name":"fdcb.o"}}

#include "./fdcb.h"

#include <unistd.h>

#include <thread>
#include <cstring>

namespace
{
	struct fd_wrapper
	{
		fd_wrapper():value{-1}{}
		fd_wrapper(int fd) : value{fd} {}
		fd_wrapper(std::nullptr_t) : value{-1} {}

		operator int() const {return value;}

		bool operator ==(const fd_wrapper& other) const = default;
		bool operator !=(const fd_wrapper& other) const = default;
		bool operator ==(std::nullptr_t) const {return value == -1;}
		bool operator !=(std::nullptr_t) const {return value != -1;}

		int value;
	};

	struct fd_deleter
	{
		using pointer = fd_wrapper;

		void operator()(fd_wrapper fd) const
		{
			if(fd != nullptr)
			{
				close(fd);
			}
		}
	};

	using fd_handle = std::unique_ptr<int, fd_deleter>;

	thread_local std::string error_message;
}

struct fdcb_context
{
public:
	explicit fdcb_context(int fd, fdcb_callback callback, void* user_context)
	{
		int fds[2];
		if(pipe(fds) == -1)
		{
			auto const errstring = strerror(errno);
			throw std::runtime_error{std::string{"fdcb: Failed to create pipe: "}.append(errstring)};
		}
		m_read_fd = fd_handle{fds[0]};
		m_write_fd = fd_handle{fds[1]};

		auto old_fd = dup(fd);
		if(old_fd == -1)
		{
			auto const errstring = strerror(errno);
			throw std::runtime_error{std::string{"fdcb: Failed to dup: "}.append(errstring)};
		}
		m_old_fd = fd_handle{old_fd};

		if(dup2(m_write_fd.get(), fd) == -1)
		{
			auto const errstring = strerror(errno);
			throw std::runtime_error{std::string{"fdcb: Failed to dup: "}.append(errstring)};
		}
	}

	void flush()
	{}

private:
	std::jthread m_writer;
	fd_handle m_read_fd;
	fd_handle m_write_fd;
	fd_handle m_old_fd;
};

struct fdcb_context* fdcb_create_context(int fd, fdcb_callback callback, void* user_context)
{
	try
	{
		return new fdcb_context{fd, callback, user_context};
	}
	catch(std::exception const& error)
	{
		error_message = error.what();
		return nullptr;
	}
}

void fdcb_flush(struct fdcb_context* ctxt)
{
	ctxt->flush();
}

void fdcb_free_context(fdcb_context* ctxt)
{
	delete ctxt;
}