/***********************************
** Tsunagari Tile Engine          **
** pack-file.cpp                  **
** Copyright 2016 Paul Merrill    **
***********************************/

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

#include "pack/pack-file.h"

#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/uio.h>
#include <unistd.h>

#include <algorithm>
#include <string>
#include <unordered_map>
#include <vector>

                                       // T   s    u    n    a   g    a   r
static constexpr uint8_t PACK_MAGIC[8] = {84, 115, 117, 110, 97, 103, 97, 114};

struct Header {
    uint8_t magic[8];
    uint64_t pathOffsetsOffset;
    uint64_t pathsOffset;
    uint64_t metadataOffset;
    uint64_t dataOffset;
    uint32_t blobCount;
};

typedef uint64_t PathOffset;

enum BlobCompressionType {
    BLOB_COMPRESSION_NONE
};

struct BlobMetadata {
    BlobSize uncompressedSize;
    BlobSize compressedSize;
    BlobCompressionType compressionType;
};

struct Blob {
    std::string path;
    BlobSize size;
    const void* data;
};

static bool operator<(const Blob& a, const Blob& b) {
    return a.size < b.size;
}


class PackReaderImpl : public PackReader {
 public:
    FileIndex size();

    FileIndex findIndex(const std::string& path);

    std::string getPath(FileIndex index);
    uint64_t getSize(FileIndex index);
    const void* getData(FileIndex index);
};

std::unique_ptr<PackReader> PackReader::fromFile(const std::string& path) {
    return std::make_unique<PackReaderImpl>();
}

PackReader::FileIndex PackReaderImpl::size() {
    return 0;
}

PackReader::FileIndex PackReaderImpl::findIndex(const std::string& path) {
    return 0;
}

std::string PackReaderImpl::getPath(PackReader::FileIndex index) {
    return "";
}

uint64_t PackReaderImpl::getSize(PackReader::FileIndex index) {
    return 0;
}

const void* PackReaderImpl::getData(PackReader::FileIndex index) {
    return nullptr;
}


class PackWriterImpl : public PackWriter {
 public:
    bool writeToFile(const std::string& path);

    void addBlob(std::string path, uint64_t size, const void* data);

 private:
    std::vector<Blob> blobs;
    bool sorted = true;
};

std::unique_ptr<PackWriter> PackWriter::make() {
    return std::make_unique<PackWriterImpl>();
}

bool PackWriterImpl::writeToFile(const std::string &path) {
    // Sort blobs by size (smallest first).
    if (!sorted) {
        sorted = true;
        std::sort(blobs.begin(), blobs.end());
    }

    // Determine block offsets.
    uint64_t pathOffsetsSize = blobs.size() * sizeof(PathOffset);
    uint64_t pathsSize = 0;
    uint64_t metadataSize = blobs.size() * sizeof(BlobMetadata);
    uint64_t dataSize = 0;

    for (auto& blob : blobs) {
        pathsSize += blob.path.size();
        dataSize += blob.size;
    }

    // Construct blocks.
    Header header = {
            {PACK_MAGIC[0], PACK_MAGIC[1], PACK_MAGIC[2], PACK_MAGIC[3],
            PACK_MAGIC[4], PACK_MAGIC[5], PACK_MAGIC[6], PACK_MAGIC[7]},
            sizeof(Header),
            sizeof(Header) + pathOffsetsSize,
            sizeof(Header) + pathOffsetsSize + pathsSize,
            sizeof(Header) + pathOffsetsSize + pathsSize + metadataSize,
            static_cast<uint32_t>(blobs.size())
    };

    std::vector<PathOffset> pathOffsets(blobs.size());
    std::string paths;
    std::vector<BlobMetadata> metadatas(blobs.size());
    std::vector<iovec> ios;

    PathOffset pathOffset = header.pathsOffset;
    for (auto& blob : blobs) {
        pathOffsets.push_back(pathOffset);
        pathOffset += blob.path.size();
    }

    for (auto& blob : blobs) {
        paths += blob.path;
    }

    for (auto& blob : blobs) {
        BlobMetadata metadata = {
                blob.size,
                blob.size,
                BLOB_COMPRESSION_NONE
        };
        metadatas.push_back(metadata);
    }

    // Build IOs.
    ios.push_back({&header, sizeof(header)});
    ios.push_back({pathOffsets.data(), pathOffsetsSize});
    ios.push_back({const_cast<char*>(paths.data()), pathsSize});
    ios.push_back({metadatas.data(), metadataSize});
    for (auto& blob : blobs) {
        ios.push_back({const_cast<void*>(blob.data), blob.size});
    }

    // Write file.
    int fd = open(path.c_str(), O_CREAT | O_WRONLY | O_TRUNC);
    writev(fd, ios.data(), static_cast<int>(ios.size()));
    close(fd);

    return true;
}

void PackWriterImpl::addBlob(std::string path, uint64_t size, const void *data) {
    blobs.push_back({path, size, data});
    sorted = false;
}
