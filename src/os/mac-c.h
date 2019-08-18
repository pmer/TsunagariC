/********************************
** Tsunagari Tile Engine       **
** os/mac-c.h                  **
** Copyright 2019 Paul Merrill **
********************************/

// **********
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.
// **********

#ifndef SRC_OS_MAC_C_H_
#define SRC_OS_MAC_C_H_

#include "util/int.h"

// sys/_types.h
extern "C" {
typedef int64_t blkcnt_t;
typedef int32_t blksize_t;
typedef int32_t dev_t;
typedef uint32_t gid_t;
typedef uint64_t ino64_t;
typedef uint16_t mode_t;
typedef uint16_t nlink_t;
typedef int64_t off_t;
typedef uint32_t uid_t;
struct iovec {
    void* iov_base;
    size_t iov_len;
};
struct timespec {
    time_t tv_sec;
    long tv_nsec;
};
}

// sys/_pthread/_pthread_types.h
extern "C" {
#define __PTHREAD_MUTEX_SIZE__ 56
#define __PTHREAD_COND_SIZE__ 40
#define __PTHREAD_SIZE__ 8176
struct pthread_mutex_t {
    long __sig;
    char __opaque[__PTHREAD_MUTEX_SIZE__];
};
struct pthread_cond_t {
    long __sig;
    char __opaque[__PTHREAD_COND_SIZE__];
};
struct _pthread_t {
    long __sig;
    struct __darwin_pthread_handler_rec* __cleanup_stack;
    char __opaque[__PTHREAD_SIZE__];
};
typedef _pthread_t* pthread_t;
}

// sys/dirent.h
extern "C" {
#define __DARWIN_MAXPATHLEN 1024
struct dirent {
    uint64_t d_ino;
    uint64_t d_seekoff;
    uint16_t d_reclen;
    uint16_t d_namlen;
    uint8_t d_type;
    char d_name[__DARWIN_MAXPATHLEN];
};
#define DT_DIR 4
#define DT_REG 8
}

// sys/errno.h
extern "C" {
extern int* __error() noexcept;
#define errno (*__error())
#define EINTR 4
}

// sys/fcntl.h
extern "C" {
int open(const char*, int, ...) noexcept;
#define O_RDONLY 0x0000
#define O_WRONLY 0x0001
#define O_CREAT 0x0200
#define O_TRUNC 0x0400
}

// sys/mman.h
extern "C" {
void* mmap(void*, size_t, int, int, int, off_t) noexcept;
int munmap(void*, size_t) noexcept;
#define MAP_FAILED ((void*)-1)
#define MAP_SHARED 0x0001
#define PROT_READ 0x01
}

// sys/stat.h
extern "C" {
struct stat {
    dev_t st_dev;
    mode_t st_mode;
    nlink_t st_nlink;
    ino64_t st_ino;
    uid_t st_uid;
    gid_t st_gid;
    dev_t st_rdev;
    struct timespec st_atimespec;
    struct timespec st_mtimespec;
    struct timespec st_ctimespec;
    struct timespec st_birthtimespec;
    off_t st_size;
    blkcnt_t st_blocks;
    blksize_t st_blksize;
    uint32_t st_flags;
    uint32_t st_gen;
    int32_t st_lspare;
    int64_t st_qspare[2];
};
int fstat(int, struct stat*) noexcept __asm("_fstat$INODE64");
int mkdir(const char*, mode_t) noexcept;
int stat(const char*, struct stat*) noexcept __asm("_stat$INODE64");
#define S_IFMT 0170000
#define S_IFDIR 0040000
#define S_ISDIR(m) (((m)&S_IFMT) == S_IFDIR)
}

// sys/sysctl.h
extern "C" {
int sysctl(int*, unsigned int, void*, size_t*, void*, size_t) noexcept;
#define CTL_HW 6
#define HW_NCPU 3
}

// sys/uio.h
extern "C" {
ssize_t writev(int, const struct iovec*, int) noexcept;
}

// _stdio.h
extern "C" {
struct FILE;
}

// dirent.h
extern "C" {
struct DIR {
    int __dd_fd;
    long __dd_loc;
    long __dd_size;
    char* __dd_buf;
    int __dd_len;
    long __dd_seek;
    long __padding;
    int __dd_flags;
    pthread_mutex_t __dd_lock;
    struct _telldir* __dd_td;
};
int closedir(DIR*) noexcept;
DIR* opendir(const char*) noexcept __asm("_opendir$INODE64");
struct dirent* readdir(DIR*) noexcept __asm("_readdir$INODE64");
}

// malloc/_malloc.h
extern "C" {
void free(void*) noexcept;
void* malloc(size_t) noexcept;
}

// math.h
extern "C" {
double atan2(double, double) noexcept;
double ceil(double) noexcept;
float ceilf(float) noexcept;
double cos(double) noexcept;
double floor(double) noexcept;
double sin(double) noexcept;
double sqrt(double) noexcept;
}

// pthread/pthread.h
// pthread/pthread_impl.h
extern "C" {
#define _PTHREAD_MUTEX_SIG_init 0x32AAABA7
#define PTHREAD_MUTEX_INITIALIZER      \
    {                                  \
        _PTHREAD_MUTEX_SIG_init, { 0 } \
    }
#define _PTHREAD_COND_SIG_init 0x3CB0B1BB
#define PTHREAD_COND_INITIALIZER      \
    {                                 \
        _PTHREAD_COND_SIG_init, { 0 } \
    }
int pthread_mutex_destroy(pthread_mutex_t*) noexcept;
int pthread_mutex_lock(pthread_mutex_t*) noexcept;
int pthread_mutex_unlock(pthread_mutex_t*) noexcept;
int pthread_cond_destroy(pthread_cond_t*) noexcept;
int pthread_cond_signal(pthread_cond_t*) noexcept;
int pthread_cond_broadcast(pthread_cond_t*) noexcept;
int pthread_cond_wait(pthread_cond_t*, pthread_mutex_t*) noexcept;
int pthread_create(pthread_t*, const void*, void* (*)(void*), void*) noexcept;
int pthread_join(pthread_t, void**) noexcept;
}

// stdio.h
extern "C" {
int fclose(FILE*) noexcept;
FILE* fopen(const char*, const char*) noexcept;
int fprintf(FILE*, const char*, ...) noexcept;
size_t fread(void*, size_t, size_t, FILE*) noexcept;
int printf(const char*, ...) noexcept;
int sprintf(char*, const char*, ...) noexcept;
extern FILE* __stdinp;
extern FILE* __stdoutp;
extern FILE* __stderrp;
#define stdin __stdinp
#define stdout __stdoutp
#define stderr __stderrp
}

// stdlib.h
extern "C" {
int atoi(const char*) noexcept;
void exit(int) noexcept;
int rand() noexcept;
void srand(unsigned) noexcept;
double strtod(const char*, char**) noexcept;
long strtol(const char*, char**, int) noexcept;
unsigned long strtoul(const char*, char**, int) noexcept;
}

// string.h
extern "C" {
void* memchr(const void*, int, size_t) noexcept;
int memcmp(const void*, const void*, size_t) noexcept;
void *memcpy(void*, const void*, size_t) noexcept;
void* memmem(const void*, size_t, const void*, size_t) noexcept;
size_t strlen(char const*) noexcept;
}

// time.h
extern "C" {
enum clockid_t {
    CLOCK_UPTIME_RAW = 8,
};
int clock_gettime(clockid_t, struct timespec*) noexcept;
int nanosleep(const struct timespec*, struct timespec*) noexcept;
time_t time(time_t*) noexcept;
}

// unistd.h
extern "C" {
int close(int) noexcept;
int isatty(int) noexcept;
ssize_t write(int, const void*, size_t) noexcept;
}

#endif  // SRC_OS_MAC_C_H_
