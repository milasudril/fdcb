//@	{"target":{"name":"fdcb.o"}}

#include "./fdcb.h"

#include <unistd.h>

#include <thread>
#include <cstring>

namespace fdcb
{
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

	class fd_swap
	{
	public:
		fd_swap():m_replaced_fd{-1}{}
		fd_swap(fd_swap&&) = delete;
		fd_swap& operator=(fd_swap&&) = delete;

		explicit fd_swap(int replacing_fd, int fd_to_replace):
			m_replaced_fd{fd_to_replace}
		{
			auto const replaced_fd_copy = dup(fd_to_replace);
			if(replaced_fd_copy == -1)
			{
				auto const errstring = strerror(errno);
				throw std::runtime_error{std::string{"fdcb: Failed to dup: "}.append(errstring)};
			}
			m_replaced_fd_copy = fd_handle{replaced_fd_copy};

			if(dup2(replacing_fd, fd_to_replace) == -1)
			{
				auto const errstring = strerror(errno);
				throw std::runtime_error{std::string{"fdcb: Failed to dup: "}.append(errstring)};
			}
		}

		~fd_swap()
		{
			if(dup2(m_replaced_fd_copy.get(), m_replaced_fd) == -1)
			{
				abort();
			}
		}

	private:
		int m_replaced_fd;
		fd_handle m_replaced_fd_copy;
	};

	class pipe
	{
	public:
		pipe()
		{
			int fds[2];
			if(::pipe(fds) == -1)
			{
				auto const errstring = strerror(errno);
				throw std::runtime_error{std::string{"fdcb: Failed to create pipe: "}.append(errstring)};
			}
			m_read_fd = fd_handle{fds[0]};
			m_write_fd = fd_handle{fds[1]};
		}

		auto input() const { return m_write_fd.get(); }

		auto output() const { return m_read_fd.get(); }

	private:
		fd_handle m_read_fd;
		fd_handle m_write_fd;
	};
}
}

struct fdcb_context
{
public:
	explicit fdcb_context(int fd, fdcb_callback callback, void* user_context):
	 m_fd_swap{m_pipe.input(), fd}
	{
	}

	void flush()
	{}

private:
	std::jthread m_writer;
	fdcb::pipe m_pipe;
	fdcb::fd_swap m_fd_swap;
};

fdcb_context* fdcb_create_context(int fd, fdcb_callback callback, void* user_context)
{
	try
	{
		return new fdcb_context{fd, callback, user_context};
	}
	catch(std::exception const& error)
	{
		fdcb::error_message = error.what();
		return nullptr;
	}
}

char const* fdcb_get_error_message()
{
	return fdcb::error_message.c_str();
}

void fdcb_flush(struct fdcb_context* ctxt)
{
	ctxt->flush();
}

void fdcb_free_context(fdcb_context* ctxt)
{
	delete ctxt;
}