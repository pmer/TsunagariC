/***************************************
** Tsunagari Tile Engine              **
** os/windows.h                       **
** Copyright 2011-2013 Michael Reiley **
** Copyright 2011-2019 Paul Merrill   **
***************************************/

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

#ifndef SRC_OS_WINDOWS_H_
#define SRC_OS_WINDOWS_H_

#include <string>

#define UNICODE
#include <Windows.h>

#include "util/optional.h"

class MappedFile {
 public:
    static Optional<MappedFile> fromPath(const std::string& path);

    MappedFile();
    MappedFile(MappedFile&& other);
    MappedFile(const MappedFile& other) = delete;
    ~MappedFile();

    MappedFile& operator=(MappedFile&& other);

    template<typename T>
    const T at(size_t offset) const {
        return reinterpret_cast<T>(data + offset);
    }

 private:
    HANDLE file;
    HANDLE mapping;
    char* data;
};

/* Visual C++ ignorantly assumes that all programs will use either a console OR
 * a window. Our program needs a window and an optional console. When Tsunagari
 * is run from the command line, this function forces Windows to reattach a
 * console to its process. Otherwise it does nothing.
 */
void wFixConsole();

// Simple wrapper to create a halting (modal) message box.
void wMessageBox(const std::string& title, const std::string& text);

#endif  // SRC_OS_WINDOWS_H_
