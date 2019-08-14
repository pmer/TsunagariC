/*************************************
** Tsunagari Tile Engine            **
** pack-writer.h                    **
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

#ifndef SRC_PACK_PACK_WRITER_H_
#define SRC_PACK_PACK_WRITER_H_

#include "util/int.h"
#include "util/noexcept.h"
#include "util/string-view.h"
#include "util/string.h"
#include "util/unique.h"

class PackWriter {
 public:
    typedef uint32_t BlobSize;

    static Unique<PackWriter> make() noexcept;
    virtual ~PackWriter() = default;

    virtual bool writeToFile(StringView path) noexcept = 0;

    virtual void addBlob(String path, BlobSize size, const void* data) noexcept = 0;
};

#endif  // SRC_PACK_PACK_WRITER_H_
