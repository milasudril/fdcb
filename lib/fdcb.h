//@	{"dependencies_extra":[{"ref":"./fdcb.o", "rel":"implementation"}]}

#ifndef FDCB_H
#define FDCB_H

#ifdef __cplusplus
#define BEGIN_C extern "C" {
#define END_C }
#else
#define BEGIN_C
#define END_C
#endif

#include <sys/types.h>

BEGIN_C

typedef size_t (*fdcb_callback)(void* user_context, void const* buffer, size_t count);

struct fdcb_context;

struct fdcb_context* fdcb_create_context(int fd, void* user_context, fdcb_callback callback);

void fdcb_free_context(struct fdcb_context*);

char const* fdcb_get_error_message();

END_C

#ifdef __cplusplus

#include <memory>
#include <span>
#include <cstddef>

namespace fdcb
{
	struct fdcb_context_deleter
	{
		void operator()(fdcb_context* ptr) const
		{
			fdcb_free_context(ptr);
		}
	};

	using context_handle = std::unique_ptr<fdcb_context, fdcb_context_deleter>;

	template<class Writer>
	requires(!std::is_pointer_v<Writer> && !std::is_reference_v<Writer>)
	class context
	{
	private:
		static size_t call_write(void* user_context, void const* buffer, size_t count)
		{
			auto bytes = std::span{reinterpret_cast<std::byte const*>(buffer), count};
			return write(*static_cast<Writer*>(user_context), bytes);
		}

	public:
		explicit context(int fd, Writer&& cb):
			m_cb{std::make_unique<Writer>(std::move(cb))},
			m_context{fdcb_create_context(fd, m_cb.get(), call_write)}
		{
			if(m_context == nullptr)
			{
				throw std::runtime_error{fdcb_get_error_message()};
			}
		}

	private:
		std::unique_ptr<Writer> m_cb;  // Use a unique_ptr to avoid issue of moving contexts
		context_handle m_context;
	};
}
#endif

#endif