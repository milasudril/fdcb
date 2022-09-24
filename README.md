# FdCb

File Descriptor Callback library

FdCb is a library that allows association of a callback with writes to a file descriptor. This makes
it possible to reroute the output of `printf` to a GUI or a logging system. For example usage, see
lib/fdcb.test.cpp.

To provide a stable ABI, the library has a PIMPL C interface. To make it easier to use, there is
also a templated C++ wrapper.