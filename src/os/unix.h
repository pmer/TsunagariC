/**********************************
** Tsunagari Tile Engine         **
** os/unix.h                     **
** Copyright 2019 Paul Merrill   **
**********************************/

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

#ifndef SRC_OS_UNIX_H_
#define SRC_OS_UNIX_H_

#include "util/int.h"
#include "util/optional.h"
#include "util/string-view.h"
#include "util/string.h"

class MappedFile {
 public:
    static Optional<MappedFile> fromPath(String& path) noexcept;
    static Optional<MappedFile> fromPath(StringView path) noexcept;

    MappedFile() noexcept;
    MappedFile(MappedFile&& other) noexcept;
    MappedFile(const MappedFile& other) = delete;
    MappedFile(char* map, size_t len) noexcept;
    ~MappedFile() noexcept;

    MappedFile& operator=(MappedFile&& other) noexcept;

    template<typename T> const T at(size_t offset) const noexcept {
        return reinterpret_cast<T>(map + offset);
    }

 private:
    char* map;
    size_t len;
};

#endif  // SRC_OS_UNIX_H_
