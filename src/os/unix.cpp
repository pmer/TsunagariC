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

#include "os/os.h"

#include "os/c.h"
#include "os/unix-mutex.h"
#include "util/vector.h"

const char dirSeparator = '/';

Filesize
getFileSize(StringView path_) noexcept {
    String path(path_);

	struct stat status;
    if (stat(path.null(), &status)) {
        return mark;
    }
    return Filesize(static_cast<uint64_t>(status.st_size));
}

bool
isDir(String& path) noexcept {
    struct stat status;
    if (stat(path.null(), &status)) {
        return false;
    }
    return S_ISDIR(status.st_mode);
}

bool
isDir(StringView path) noexcept {
    String path_(path);
    return isDir(path_);
}

void
makeDirectory(String& path) noexcept {
    mkdir(path.null(), 0777);
}

void
makeDirectory(StringView path) noexcept {
    String path_(path);
    return makeDirectory(path_);
}

Vector<String>
listDir(String& path) noexcept {
    DIR* dir = nullptr;
    struct dirent* entry = nullptr;
    Vector<String> names;

    if ((dir = opendir(path.null())) == nullptr) {
        return names;
    }

    // FIXME: Replace with reentrant function calls.
    while ((entry = readdir(dir)) != nullptr) {
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
        names.push_back(entry->d_name);
    }

    closedir(dir);

    return names;
}

Vector<String>
listDir(StringView path) noexcept {
    String path_(path);
    return listDir(path_);
}

bool
writeFile(String& path, uint32_t length, void* data) noexcept {
    int fd = open(path.null(), O_CREAT | O_WRONLY | O_TRUNC, 0666);
    if (fd == -1) {
        return false;
    }
    ssize_t written = write(fd, data, length);
    if (written != length) {
        close(fd);
        return false;
    }
    close(fd);
    return true;
}

bool
writeFile(StringView path, uint32_t length, void* data) noexcept {
    String path_(path);
    return writeFile(path_, length, data);
}

bool
writeFileVec(String& path, uint32_t count, uint32_t* lengths, void** datas) noexcept {
    int fd = open(path.null(), O_CREAT | O_WRONLY | O_TRUNC, 0666);
    if (fd == -1) {
        return false;
    }

    ssize_t total = 0;
    Vector<iovec> ios;

    ios.reserve(count);
    for (size_t i = 0; i < count; i++) {
        total += lengths[i];
        ios.push_back({datas[i], lengths[i]});
    }

    ssize_t written = writev(fd, ios.data(), static_cast<int>(ios.size()));
    if (written != total) {
        close(fd);
        return false;
    }

    close(fd);
    return true;
}

bool
writeFileVec(StringView path, uint32_t count, uint32_t* lengths, void** datas) noexcept {
    String path_(path);
    return writeFileVec(path_, count, lengths, datas);
}

/*
bool writeFileVec(String& path, uint32_t count, uint32_t* lengths, void** datas) noexcept {
    int fd = open(path.null(), O_CREAT | O_WRONLY | O_TRUNC, 0666);
    if (fd == -1) {
        return false;
    }
    for (size_t i = 0; i < count; i++) {
        size_t length = lengths[i];
        void* data = datas[i];
        ssize_t written = write(fd, data, length);
        if (written != length) {
            close(fd);
            return false;
        }
    }
    close(fd);
    return true;
}
*/

Optional<String>
readFile(String& path) noexcept {
    Filesize size = getFileSize(path);
    if (!size) {
        return Optional<String>();
    }

    FILE* f = fopen(path.null(), "r");
    if (!f) {
        return Optional<String>();
    }

    String contents;
    contents.resize(*size);

    ssize_t read = fread(contents.data(), *size, 1, f);
    if (read != 1) {
        fclose(f);
        return Optional<String>();
    }

    fclose(f);

    return Optional<String>(move_(contents));
}

Optional<String>
readFile(StringView s) noexcept {
    String s_(s);
    return readFile(s_);
}

static bool
isaTTY() noexcept {
    static bool checked = false;
    static bool tty = false;

    if (!checked) {
        checked = true;
        tty = isatty(0) != 0;
    }
    return tty;
}

void
setTermColor(TermColor color) noexcept {
    if (!isaTTY()) {
        return;
    }

    // VT100 terminal control codes from:
    //   http://www.termsys.demon.co.uk/vtansi.htm

    const char escape = 27;

    switch (color) {
    case TC_RESET:
        printf("%c[0m", escape);
        break;
    case TC_GREEN:
        printf("%c[32m", escape);
        break;
    case TC_YELLOW:
        printf("%c[33m", escape);
        break;
    case TC_RED:
        printf("%c[31m", escape);
        break;
    }
}
