/********************************
** Tsunagari Tile Engine       **
** os/netbsd-c.h               **
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

#ifndef SRC_OS_NETBSD_C_H_
#define SRC_OS_NETBSD_C_H_

#include "util/int.h"

// amd64/types.h
// sys/ansi.h
// sys/types.h
extern "C" {
typedef int64_t blkcnt_t;
typedef int32_t blksize_t;
typedef int clockid_t;
typedef uint64_t dev_t;
typedef uint32_t gid_t;
typedef uint64_t ino_t;
typedef uint32_t mode_t;
typedef uint32_t nlink_t;
typedef int64_t off_t;
typedef int32_t pid_t;
typedef uint32_t uid_t;
}

// sys/dirent.h
extern "C" {
struct dirent {
    ino_t d_fileno;
    uint16_t d_reclen;
    uint16_t d_namlen;
    uint8_t d_type;
    char d_name[512];
};
#define DT_DIR 4
#define DT_REG 8
}

// sys/timespec.h
extern "C" {
struct timespec {
    time_t tv_sec;
    long tv_nsec;
};
}

// sys/uio.h
extern "C" {
struct iovec {
    void* iov_base;
    size_t iov_len;
};
ssize_t writev(int, const struct iovec*, int) noexcept;
}

// sys/errno.h
// errno.h
extern "C" {
int* __errno() noexcept;
#define errno (*__errno())
#define EINTR 4
}

// fcntl.h
extern "C" {
int open(const char*, int, ...) noexcept;
#define O_RDONLY 0x00000000
#define O_WRONLY 0x00000001
#define O_CREAT 0x00000200
#define O_TRUNC 0x00000400
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
    ino_t st_ino;
    nlink_t st_nlink;
    uid_t st_uid;
    gid_t st_gid;
    dev_t st_rdev;
    struct timespec st_atim;
    struct timespec st_mtim;
    struct timespec st_ctim;
    struct timespec st_birthtim;
    off_t st_size;
    blkcnt_t st_blocks;
    blksize_t st_blksize;
    uint32_t st_flags;
    uint32_t st_gen;
    uint32_t st_spare[2];
};
int fstat(int, struct stat*) noexcept __asm("__fstat50");
int mkdir(const char*, mode_t) noexcept;
int stat(const char*, struct stat*) noexcept __asm("__stat50");
#define S_IFMT 0170000
#define S_IFDIR 0040000
#define S_ISDIR(m) (((m) & S_IFMT) == S_IFDIR)
}

// dirent.h
extern "C" {
struct _dirdesc;
typedef struct _dirdesc DIR;
int closedir(DIR*) noexcept;
DIR* opendir(const char*) noexcept __asm("__opendir30");
struct dirent* readdir(DIR*) noexcept __asm("__readdir30");
#define d_ino d_fileno
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

// amd64/types.h
// pthread_types.h
// pthread.h
extern "C" {
typedef unsigned char __pthread_spin_t;
struct __pthread_st;
typedef struct __pthread_st* pthread_t;
struct pthread_queue_t {
    struct __pthread_st* ptqh_first;
    struct __pthread_st** ptqh_last;
};
struct pthread_mutex_t {
    unsigned int ptm_magic;
    __pthread_spin_t ptm_errorcheck;
    uint8_t ptm_pad1[3];
    union {
        unsigned char ptm_ceiling;
        __pthread_spin_t ptm_unused;
    };
    uint8_t ptm_pad2[3];
    pthread_t ptm_owner;
    pthread_t* ptm_waiters;
    unsigned int ptm_recursed;
    void* ptm_spare2;
};
struct pthread_cond_t {
    unsigned int ptc_magic;
    __pthread_spin_t ptc_lock;
    pthread_queue_t ptc_waiters;
    pthread_mutex_t* ptc_mutex;
    void* ptc_private;
};
int __libc_mutex_destroy(pthread_mutex_t*) noexcept;
int __libc_mutex_lock(pthread_mutex_t*) noexcept;
int __libc_mutex_unlock(pthread_mutex_t*) noexcept;
int __libc_cond_destroy(pthread_cond_t*) noexcept;
int __libc_cond_signal(pthread_cond_t*) noexcept;
int __libc_cond_broadcast(pthread_cond_t*) noexcept;
int __libc_cond_wait(pthread_cond_t*, pthread_mutex_t*) noexcept;
int pthread_create(pthread_t*, const void*, void* (*)(void*), void*) noexcept;
int pthread_join(pthread_t, void**) noexcept;
#define PTHREAD_MUTEX_INITIALIZER { \
        0x33330003, \
        0, \
        {0, 0, 0}, \
        {0}, \
        {0, 0, 0}, \
        nullptr, \
        nullptr, \
        0, \
        nullptr \
    }
#define PTHREAD_COND_INITIALIZER { \
        0x55550005, \
        0, \
        {nullptr, nullptr}, \
        nullptr, \
        nullptr \
    }
#define pthread_mutex_destroy __libc_mutex_destroy
#define pthread_mutex_lock __libc_mutex_lock
#define pthread_mutex_unlock __libc_mutex_unlock
#define pthread_cond_destroy __libc_cond_destroy
#define pthread_cond_signal __libc_cond_signal
#define pthread_cond_broadcast __libc_cond_broadcast
#define pthread_cond_wait __libc_cond_wait
}

// stdio.h
extern "C" {
struct __sbuf {
    unsigned char* _base;
    int _size;
};
struct FILE {
    unsigned char* _p;
    int _r;
    int _w;
    unsigned short _flags;
    short _file;
    struct __sbuf _bf;
    int _lbfsize;
    void* _cookie;
    int (*_close)(void*);
    ssize_t (*_read)(void*, void*, size_t);
    off_t (*_seek)(void*, off_t, int);
    ssize_t (*_write)(void*, const void*, size_t);
    struct __sbuf _ext;
    unsigned char *_up;
    int _ur;
    unsigned char _ubuf[3];
    unsigned char _nbuf[1];
    int (*_flush)(void*);
    char _lb_unused[sizeof(struct __sbuf) - sizeof(int (*)(void*))];
    int _blk_size;
    off_t _offset;
};
int fclose(FILE*) noexcept;
FILE* fopen(const char*, const char*) noexcept;
int fprintf(FILE*, const char*, ...) noexcept;
size_t fread(void*, size_t, size_t, FILE*) noexcept;
int printf(const char*, ...) noexcept;
int sprintf(char*, const char*, ...) noexcept;
extern FILE __sF[3];
#define stdin (&__sF[0])
#define stdout (&__sF[1])
#define stderr (&__sF[2])
}

// stdlib.h
extern "C" {
int atoi(const char*) noexcept;
void exit(int) noexcept;
void free(void*) noexcept;
void* malloc(size_t) noexcept;
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
void* memmem(const void*, size_t, const void*, size_t) noexcept;
size_t strlen(char const*) noexcept;
}

// sys/time.h
// time.h
extern "C" {
int clock_gettime(clockid_t, struct timespec*) noexcept;
int nanosleep(const struct timespec*, struct timespec*) noexcept;
time_t time(time_t*) noexcept;
#define CLOCK_MONOTONIC 3
}

// sys/unistd.h
// unistd.h
extern "C" {
int close(int) noexcept;
int isatty(int) noexcept;
long sysconf(int) noexcept;
ssize_t write(int, const void*, size_t) noexcept;
#define _SC_NPROCESSORS_ONLN 1002
}

#endif  // SRC_OS_NETBSD_C_H_
