//@	{"target":{"name":"fdcb.o"}}

#include "./fdcb.h"

struct fdcb_context
{
	explicit fdcb_context(int fd, fdcb_callback callback, void* user_context)
	{}

	void flush()
	{}
};

struct fdcb_context* fdcb_create_context(int fd, fdcb_callback callback, void* user_context)
{
	return new fdcb_context{fd, callback, user_context};
}

void fdcb_flush(struct fdcb_context* ctxt)
{
	ctxt->flush();
}

void fdcb_free_context(fdcb_context* ctxt)
{
	delete ctxt;
}