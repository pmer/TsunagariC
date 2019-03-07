/*************************************
** Tsunagari Tile Engine            **
** pack-writer.cpp                  **
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

#include "pack/pack-writer.h"

#include <stdint.h>

#include "os/os.h"
#include "pack/file-type.h"
#include "pack/pack-reader.h"
#include "util/sort.h"
#include "util/string.h"
#include "util/vector.h"

//                                       "T   s    u    n    a   g    a   r"
static constexpr uint8_t PACK_MAGIC[8] = {84, 115, 117, 110, 97, 103, 97, 114};

static constexpr uint8_t PACK_VERSION = 1;

struct HeaderBlock {
    uint8_t magic[8];
    uint8_t version;
    uint8_t unused[7];
    uint64_t blobCount;
    uint64_t pathOffsetsBlockOffset;
    uint64_t pathsBlockOffset;
    uint64_t pathsBlockSize;
    uint64_t metadataBlockOffset;
    uint64_t dataOffsetsBlockOffset;
};

typedef uint64_t PathOffset;

enum BlobCompressionType {
    BLOB_COMPRESSION_NONE
};

struct BlobMetadata {
    PackReader::BlobSize uncompressedSize;
    PackReader::BlobSize compressedSize;
    BlobCompressionType compressionType;
};

struct Blob {
    String path;
    PackWriter::BlobSize size;
    const void* data;
};

static bool operator<(const Blob& a, const Blob& b) {
    FileType typeA = determineFileType(a.path);
    FileType typeB = determineFileType(b.path);
    if (typeA < typeB) {
        return true;
    } else if (typeA > typeB) {
        return false;
    } else {
        return a.path < b.path;
    }
}


class PackWriterImpl : public PackWriter {
 public:
    bool writeToFile(StringView path);

    void addBlob(String path, BlobSize size, const void* data);

 private:
    vector<Blob> blobs;
    bool sorted = true;
};

Unique<PackWriter> PackWriter::make() {
    return Unique<PackWriter>(new PackWriterImpl);
}

bool PackWriterImpl::writeToFile(StringView path) {
    size_t blobCount = blobs.size();

    // Sort blobs by size (smallest first).
    if (!sorted) {
        sorted = true;
        pdqsort(blobs.begin(), blobs.end());
    }

    // Determine block offsets.
    uint64_t pathOffsetsBlockSize = (blobCount + 1) * sizeof(PathOffset);
    uint64_t pathsBlockSize = 0;
    uint64_t metadataBlockSize = blobCount * sizeof(BlobMetadata);
    uint64_t dataOffsetsBlockSize = blobCount * sizeof(uint64_t);

    for (auto& blob : blobs) {
        pathsBlockSize += blob.path.size();
    }

    // Construct blocks.
    HeaderBlock headerBlock = {
        {PACK_MAGIC[0], PACK_MAGIC[1], PACK_MAGIC[2], PACK_MAGIC[3],
         PACK_MAGIC[4], PACK_MAGIC[5], PACK_MAGIC[6], PACK_MAGIC[7]},

        PACK_VERSION, {0, 0, 0, 0, 0, 0, 0},

        // blobCount
        static_cast<uint64_t>(blobCount),

        // We write blocks contiguously (well, so far we do).

        // pathOffsetsBlockOffset
        sizeof(HeaderBlock),
        // pathsBlockOffset
        sizeof(HeaderBlock) + pathOffsetsBlockSize,
        // pathsBlockSize
        pathsBlockSize,
        // metadataBlockOffset
        sizeof(HeaderBlock) + pathOffsetsBlockSize + pathsBlockSize,
        // dataOffsetsBlockOffset
        sizeof(HeaderBlock) + pathOffsetsBlockSize + pathsBlockSize + metadataBlockSize,
    };

    vector<PathOffset> pathOffsetsBlock;
    String pathsBlock;
    vector<BlobMetadata> metadatasBlock;
    vector<uint64_t> dataOffsetsBlock;

    pathOffsetsBlock.reserve(blobCount + 1);
    pathsBlock.reserve(pathsBlockSize);
    metadatasBlock.reserve(blobCount);

    PathOffset pathOffset = 0;
    for (auto& blob : blobs) {
        pathOffsetsBlock.push_back(pathOffset);
        pathOffset += blob.path.size();
    }
    pathOffsetsBlock.push_back(pathOffset);

    for (auto& blob : blobs) {
        pathsBlock << blob.path;
    }

    for (auto& blob : blobs) {
        BlobMetadata metadata = {
            blob.size,
            blob.size,
            BLOB_COMPRESSION_NONE
        };
        metadatasBlock.push_back(metadata);
    }

    // Blob data starts immediately after the data offset block.
    uint64_t dataOffset = headerBlock.dataOffsetsBlockOffset + dataOffsetsBlockSize;
    for (auto& blob : blobs) {
        dataOffsetsBlock.push_back(dataOffset);
        dataOffset += blob.size;
    }

    // Build IO vector.
    vector<size_t> writeLengths;
    vector<void*> writeDatas;

    writeLengths.reserve(5 + blobCount);
    writeDatas.reserve(5 + blobCount);

    writeLengths.push_back(sizeof(headerBlock));
    writeLengths.push_back(pathOffsetsBlockSize);
    writeLengths.push_back(pathsBlockSize);
    writeLengths.push_back(metadataBlockSize);
    writeLengths.push_back(dataOffsetsBlockSize);

    writeDatas.push_back(&headerBlock);
    writeDatas.push_back(pathOffsetsBlock.data());
    writeDatas.push_back(const_cast<char*>(pathsBlock.data()));
    writeDatas.push_back(metadatasBlock.data());
    writeDatas.push_back(dataOffsetsBlock.data());

    for (auto& blob : blobs) {
        writeLengths.push_back(blob.size);
        writeDatas.push_back(const_cast<void*>(blob.data));
    }

    // Write file.
    return writeFileVec(path, writeLengths.size(), writeLengths.data(), writeDatas.data());
}

void PackWriterImpl::addBlob(String path, BlobSize size, const void *data) {
    blobs.push_back({move_(path), size, data});
    sorted = false;
}

//#include <stdio.h>
//int main() {
//    Blob a{
//        "Hi",
//        3,
//        reinterpret_cast<const void*>(&main),
//    };
//    Blob b(move_(a));
//    return 0;
//}

//struct Blob {
//    String path;
//    PackWriter::BlobSize size;
//    const void* data;
//};
