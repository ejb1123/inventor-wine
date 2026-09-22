#define _GNU_SOURCE
#include <dlfcn.h>
#include <errno.h>
#include <fcntl.h>
#include <stdarg.h>
#include <string.h>
int open(const char *path, int flags, ...)
{
    int (*real_open)(const char *, int, ...) = dlsym(RTLD_NEXT, "open");
    mode_t mode = 0;
    if (!strcmp(path, "/dev/ntsync")) { errno = ENOENT; return -1; }
    if (flags & O_CREAT) { va_list ap; va_start(ap, flags); mode = va_arg(ap, mode_t); va_end(ap); }
    return real_open(path, flags, mode);
}
int open64(const char *path, int flags, ...)
{
    int (*real_open)(const char *, int, ...) = dlsym(RTLD_NEXT, "open64");
    mode_t mode = 0;
    if (!strcmp(path, "/dev/ntsync")) { errno = ENOENT; return -1; }
    if (flags & O_CREAT) { va_list ap; va_start(ap, flags); mode = va_arg(ap, mode_t); va_end(ap); }
    return real_open(path, flags, mode);
}
