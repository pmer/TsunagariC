/********************************
** Tsunagari Tile Engine       **
** os/unix.cpp                 **
** Copyright 2016 Paul Merrill **
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

#define _DARWIN_USE_64_BIT_INODE

#include <fcntl.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/dir.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <string>
#include <vector>

char dirSeparator = '/';

uint64_t getFileSize(const std::string& path) {
    struct stat status;
    if (stat(path.c_str(), &status)) {
        return SIZE_MAX;
    }
    return static_cast<uint64_t>(status.st_size);
}

bool isDir(const std::string& path) {
    struct stat status;
    if (stat(path.c_str(), &status)) {
        return false;
    }
    return S_ISDIR(status.st_mode);
}

std::vector<std::string> listDir(const std::string& path) {
    DIR* dir = nullptr;
    struct dirent* entry = nullptr;
    std::vector<std::string> names;

    if ((dir = opendir(path.c_str())) == nullptr) {
        return std::vector<std::string>();
    }

    while ((entry = readdir(dir))) {
        if (entry->d_ino == 0) {
            // Ignore unlinked files.
            continue;
        }
        if (entry->d_name[0] == '.') {
            // Ignore hidden files and directories.
            continue;
        }
        if ((entry->d_type & (DT_DIR | DT_REG)) == 0) {
            // Ignore odd files.
            continue;
        }
        names.emplace_back(entry->d_name);
    }

    closedir(dir);

    return names;
}
