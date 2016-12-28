/********************************
** Tsunagari Tile Engine       **
** pack-file.cpp               **
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

#include "pack/pack-file.h"

#include <fcntl.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/uio.h>
#include <unistd.h>

#include <algorithm>
#include <string>
#include <unordered_map>
#include <vector>

#include "pack/file-type.h"

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
    std::string path;
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


class PackReaderImpl : public PackReader {
 public:
    ~PackReaderImpl();

    BlobIndex size() const;

    BlobIndex findIndex(const std::string& path) const;

    std::string getBlobPath(BlobIndex index) const;
    uint64_t getBlobSize(BlobIndex index) const;
    void* getBlobData(BlobIndex index);

    int fd;
    HeaderBlock header;
    std::unique_ptr<PathOffset> pathOffsets;
    std::unique_ptr<char> paths;
    std::unique_ptr<BlobMetadata> metadatas;
    std::unique_ptr<uint64_t> dataOffsets;
    std::unordered_map<std::string, BlobIndex> lookups;
};

std::unique_ptr<PackReader> PackReader::fromFile(const std::string& path) {
    PackReaderImpl* reader = new PackReaderImpl;

    reader->fd = open(path.c_str(), O_RDONLY);

    if (reader->fd == -1) {
        return std::unique_ptr<PackReader>();
    }

    int fd = reader->fd;
    HeaderBlock& header = reader->header;

    read(fd, &header, sizeof(header));

    if (memcmp(header.magic, PACK_MAGIC, sizeof(header.magic)) != 0) {
        return std::unique_ptr<PackReader>();
    }

    if (!(header.version == PACK_VERSION)) {
        return std::unique_ptr<PackReader>();
    }

    uint64_t blobCount = reader->header.blobCount;

    reader->pathOffsets.reset(new PathOffset[blobCount + 1]);
    reader->paths.reset(new char[header.pathsBlockSize]);
    reader->metadatas.reset(new BlobMetadata[blobCount]);
    reader->dataOffsets.reset(new uint64_t[blobCount]);

    uint64_t pathOffsetsBlockSize = blobCount * sizeof(PathOffset);
    uint64_t pathsBlockSize = header.pathsBlockSize;
    uint64_t metadatasBlockSize = blobCount * sizeof(BlobMetadata);
    uint64_t dataOffsetsBlockSize = blobCount * sizeof(uint64_t);

    read(fd, reader->pathOffsets.get(), pathOffsetsBlockSize);
    read(fd, const_cast<char*>(reader->paths.get()), pathsBlockSize);
    read(fd, reader->metadatas.get(), metadatasBlockSize);
    read(fd, reader->dataOffsets.get(), dataOffsetsBlockSize);

    // Add an extra element to pathOffsets that holds the end position of the
    // last path.
    reader->pathOffsets.get()[blobCount] = pathsBlockSize;

    for (PackReader::BlobIndex i = 0; i < blobCount; i++) {
        uint64_t pathBegin = reader->pathOffsets.get()[i];
        uint64_t pathEnd = reader->pathOffsets.get()[i + 1];
        std::string blobPath = std::string(
                reader->paths.get() + pathBegin,
                reader->paths.get() + pathEnd);
        reader->lookups[blobPath] = i;
    }

    return std::unique_ptr<PackReaderImpl>(reader);
}

PackReaderImpl::~PackReaderImpl() {
    close(fd);
}

PackReader::BlobIndex PackReaderImpl::size() const {
    return header.blobCount;
}

PackReader::BlobIndex PackReaderImpl::findIndex(const std::string& path) const {
    auto it = lookups.find(path);
    if (it == lookups.end()) {
        return BLOB_NOT_FOUND;
    } else {
        return it->second;
    }
}

std::string PackReaderImpl::getBlobPath(PackReader::BlobIndex index) const {
    uint64_t begin = pathOffsets.get()[index];
    uint64_t end = pathOffsets.get()[index + 1];
    return std::string(paths.get() + begin, paths.get() + end);
}

uint64_t PackReaderImpl::getBlobSize(PackReader::BlobIndex index) const {
    return metadatas.get()[index].uncompressedSize;
}

void* PackReaderImpl::getBlobData(PackReader::BlobIndex index) {
    uint64_t size = getBlobSize(index);
    void* data = malloc(size);
    uint64_t offset = dataOffsets.get()[index];
    lseek(fd, static_cast<off_t>(offset), SEEK_SET);
    read(fd, data, size);
    return data;
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
    size_t blobCount = blobs.size();

    // Sort blobs by size (smallest first).
    if (!sorted) {
        sorted = true;
        std::sort(blobs.begin(), blobs.end());
    }

    // Determine block offsets.
    uint64_t pathOffsetsBlockSize = blobCount * sizeof(PathOffset);
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

    std::vector<PathOffset> pathOffsetsBlock;
    std::string pathsBlock;
    std::vector<BlobMetadata> metadatasBlock;
    std::vector<uint64_t> dataOffsetsBlock;
    std::vector<iovec> ios;

    pathOffsetsBlock.reserve(blobCount);
    pathsBlock.reserve(pathsBlockSize);
    metadatasBlock.reserve(blobCount);
    ios.reserve(5 + blobCount);

    PathOffset pathOffset = 0;
    for (auto& blob : blobs) {
        pathOffsetsBlock.push_back(pathOffset);
        pathOffset += blob.path.size();
    }

    for (auto& blob : blobs) {
        pathsBlock += blob.path;
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
    ios.push_back({&headerBlock, sizeof(headerBlock)});
    ios.push_back({pathOffsetsBlock.data(), pathOffsetsBlockSize});
    ios.push_back({const_cast<char*>(pathsBlock.data()), pathsBlockSize});
    ios.push_back({metadatasBlock.data(), metadataBlockSize});
    ios.push_back({dataOffsetsBlock.data(), dataOffsetsBlockSize});
    for (auto& blob : blobs) {
        ios.push_back({const_cast<void*>(blob.data), blob.size});
    }

    // Write file.
    int fd = open(path.c_str(), O_CREAT | O_WRONLY | O_TRUNC, 0666);
    writev(fd, ios.data(), static_cast<int>(ios.size()));
    close(fd);

    return true;
}

void PackWriterImpl::addBlob(std::string path, uint64_t size, const void *data) {
    blobs.push_back({path, size, data});
    sorted = false;
}
