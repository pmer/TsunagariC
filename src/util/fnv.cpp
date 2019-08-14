/********************************
** Tsunagari Tile Engine       **
** fnv.cpp                     **
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

#include "util/fnv.h"

#include "util/int.h"
#include "util/noexcept.h"

#if __SIZEOF_SIZE_T__ == 4 || (defined(_WIN32) && !defined(_WIN64))

size_t
fnvHash(const char* data, size_t size) noexcept {
    size_t hash = 0x811c9dc5;

    const uint8_t* begin = (const uint8_t*)data;
    const uint8_t* end = begin + size;

    while (begin < end) {
        hash ^= (size_t)*begin++;
        hash += (hash << 1) + (hash << 4) + (hash << 7) + (hash << 8) +
                (hash << 24);
    }
    return hash;
}

#elif __SIZEOF_SIZE_T__ == 8 || (defined(_WIN32) && defined(_WIN64))

size_t
fnvHash(const char* data, size_t size) noexcept {
    size_t hash = 0xcbf29ce484222325;

    const uint8_t* begin = (const uint8_t*)data;
    const uint8_t* end = begin + size;

    while (begin < end) {
        hash ^= (size_t)*begin++;
        hash += (hash << 1) + (hash << 4) + (hash << 5) + (hash << 7) +
                (hash << 8) + (hash << 40);
    }
    return hash;
}

#endif

template<typename T> size_t hash_(const T&) noexcept;

template<>
size_t
hash_<double>(const double& d) noexcept {
    return fnvHash(reinterpret_cast<const char*>(&d), sizeof(double));
}
