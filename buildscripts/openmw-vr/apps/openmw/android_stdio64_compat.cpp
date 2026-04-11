#include <cstdio>
#include <sys/types.h>

extern "C" FILE* fopen64(const char* filename, const char* mode)
{
    return std::fopen(filename, mode);
}

extern "C" int fseeko64(FILE* stream, off64_t offset, int whence)
{
    return fseeko(stream, static_cast<off_t>(offset), whence);
}

extern "C" off64_t ftello64(FILE* stream)
{
    return static_cast<off64_t>(ftello(stream));
}
