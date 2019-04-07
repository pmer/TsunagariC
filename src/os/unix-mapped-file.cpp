/*************************************
** Tsunagari Tile Engine            **
** os/unix.cpp                      **
** Copyright 2016-2019 Paul Merrill **
*************************************/

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

#include "os/unix-mapped-file.h"

#include "os/c.h"

Optional<MappedFile>
MappedFile::fromPath(String& path) noexcept {
    int fd = open(path.null(), O_RDONLY);
    if (fd == -1) {
        return Optional<MappedFile>();
    }

    struct stat st;
    fstat(fd, &st);

    if (st.st_size == 0) {
        close(fd);
        return Optional<MappedFile>();
    }

    // Cannot open files >4 GB on 32-bit operating systems since they will fail
    // the mmap.
    if (sizeof(long long) > sizeof(size_t)) {
        if (st.st_size > static_cast<long long>(SIZE_MAX)) {
            close(fd);
            return Optional<MappedFile>();
        }
    }

    char* map = reinterpret_cast<char*>(mmap(nullptr,
                                             static_cast<size_t>(st.st_size),
                                             PROT_READ,
                                             MAP_SHARED,
                                             fd,
                                             0));
    size_t len = static_cast<size_t>(st.st_size);

    if (map == MAP_FAILED) {
        close(fd);
        return Optional<MappedFile>();
    }

    // FIXME: Close the fd now or at destruction?
    return Optional<MappedFile>(MappedFile(map, len));
}

Optional<MappedFile>
MappedFile::fromPath(StringView path) noexcept {
    String path_(path);
    return MappedFile::fromPath(path_);
}

MappedFile::MappedFile() noexcept : map(reinterpret_cast<char*>(MAP_FAILED)), len(0) {}

MappedFile::MappedFile(MappedFile&& other) noexcept {
    *this = move_(other);
}

MappedFile::MappedFile(char* map, size_t len) noexcept : map(map), len(len) {}

MappedFile::~MappedFile() noexcept {
    if (map != MAP_FAILED) {
        munmap(map, len);
    }
}

MappedFile&
MappedFile::operator=(MappedFile&& other) noexcept {
    map = other.map;
    len = other.len;
    other.map = reinterpret_cast<char*>(MAP_FAILED);
    other.len = 0;
    return *this;
}
