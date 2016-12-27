/********************************
** Tsunagari Tile Engine       **
** pack-file.h                 **
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

#ifndef SRC_PACK_PACK_FILE_H_
#define SRC_PACK_PACK_FILE_H_

#include <stdint.h>

#include <memory>
#include <string>

typedef uint64_t BlobSize;

class PackReader {
 public:
    typedef uint64_t BlobIndex;

    static constexpr BlobIndex BLOB_NOT_FOUND = UINT64_MAX;

    static std::unique_ptr<PackReader> fromFile(const std::string& path);
    virtual ~PackReader() = default;

    virtual BlobIndex size() const = 0;

    virtual BlobIndex findIndex(const std::string& path) const = 0;

    virtual std::string getBlobPath(BlobIndex index) const = 0;
    virtual BlobSize getBlobSize(BlobIndex index) const = 0;
    virtual void* getBlobData(BlobIndex index) = 0;
};

class PackWriter {
 public:
    static std::unique_ptr<PackWriter> make();
    virtual ~PackWriter() = default;

    virtual bool writeToFile(const std::string& path) = 0;

    virtual void addBlob(std::string path, BlobSize size, const void* data) = 0;
};

#endif  // SRC_PACK_PACK_FILE_H_
