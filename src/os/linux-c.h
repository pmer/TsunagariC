/********************************
** Tsunagari Tile Engine       **
** os/linux-c.h                **
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

#ifndef SRC_OS_LINUX_C_H_
#define SRC_OS_LINUX_C_H_

#include "util/int.h"

// bits/alltypes.h
extern "C" {
typedef int64_t blkcnt_t;
typedef long blksize_t;
typedef int clockid_t;
typedef uint64_t dev_t;
typedef unsigned gid_t;
typedef uint64_t ino_t;
typedef unsigned mode_t;
typedef unsigned long nlink_t;
typedef int64_t off_t;
typedef unsigned long pthread_t;
typedef unsigned uid_t;
typedef struct _IO_FILE FILE;
struct iovec {
    void* iov_base;
    size_t iov_len;
};
struct pthread_cond_t {
    union {
        int __i[12];
        volatile int __vi[12];
        void *__p[6];
    };
};
struct pthread_mutex_t {
    union {
        int __i[10];
        volatile int __vi[10];
        volatile void *volatile __p[5];
    };
};
struct timespec {
    time_t tv_sec;
    long tv_nsec;
};
}

// bits/errno.h
// errno.h
extern "C" {
int* __errno_location() noexcept;
#define errno (*__errno_location())
#define EINTR 4
}

// bits/fcntl.h
// fcntl.h
extern "C" {
int open(const char*, int, ...) noexcept;
#define O_RDONLY 00
#define O_WRONLY 01
#define O_CREAT 0100
#define O_TRUNC 01000
}

// sys/mman.h
extern "C" {
void* mmap(void*, size_t, int, int, int, off_t) noexcept;
int munmap(void*, size_t) noexcept;
#define MAP_FAILED ((void*)-1)
#define MAP_SHARED 0x01
#define PROT_READ 1
}

// bits/stat.h
// sys/stat.h
extern "C" {
struct stat {
	dev_t st_dev;
	ino_t st_ino;
	nlink_t st_nlink;
	mode_t st_mode;
	uid_t st_uid;
	gid_t st_gid;
	unsigned int pad1;
	dev_t st_rdev;
	off_t st_size;
	blksize_t st_blksize;
	blkcnt_t st_blocks;
	struct timespec st_atim;
	struct timespec st_mtim;
	struct timespec st_ctim;
	long pad2[3];
};
int fstat(int, struct stat*) noexcept;
int mkdir(const char*, mode_t) noexcept;
int stat(const char*, struct stat*) noexcept;
#define S_IFMT 0170000
#define S_IFDIR 0040000
#define S_ISDIR(m) (((m)&S_IFMT) == S_IFDIR)
}

// sys/uio.h
extern "C" {
ssize_t writev(int, const struct iovec*, int) noexcept;
}

// dirent.h
extern "C" {
struct dirent {
    ino_t d_ino;
	off_t d_off;
	unsigned short d_reclen;
	unsigned char d_type;
	char d_name[256];
};
typedef struct __dirstream DIR;
int closedir(DIR*) noexcept;
DIR* opendir(const char*) noexcept;
struct dirent* readdir(DIR*) noexcept;
#define DT_DIR 4
#define DT_REG 8
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

// pthread.h
extern "C" {
#define PTHREAD_MUTEX_INITIALIZER {{{0}}}
#define PTHREAD_COND_INITIALIZER {{{0}}}
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
extern FILE* const stdin;
extern FILE* const stdout;
extern FILE* const stderr;
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

// time.h
extern "C" {
int clock_gettime(clockid_t, struct timespec*) noexcept;
int nanosleep(const struct timespec*, struct timespec*) noexcept;
time_t time(time_t*) noexcept;
#define CLOCK_MONOTONIC 1
}

// unistd.h
extern "C" {
int close(int) noexcept;
int isatty(int) noexcept;
long sysconf(int) noexcept;
ssize_t write(int, const void*, size_t) noexcept;
#define _SC_NPROCESSORS_ONLN 84
}

#endif  // SRC_OS_LINUX_C_H_
