/*************************************
** Tsunagari Tile Engine            **
** pack-file.cpp                    **
** Copyright 2016-2017 Paul Merrill **
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

#include "pack/pack-file.h"

#include <fcntl.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/uio.h>
#include <unistd.h>

#include <algorithm>
#include <string>
#include <unordered_map>

#include "pack/file-type.h"
#include "util/unique.h"
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

    vector<void*> getBlobDatas(vector<BlobIndex> indicies);

    void constructLookups(PackReaderImpl* reader);

    char* map;
    size_t mapLen;

    // Pointers into `map`.
    HeaderBlock* header;
    PathOffset* pathOffsets;
    char* paths;
    BlobMetadata* metadatas;
    uint64_t* dataOffsets;

    bool lookupsConstructed = false;
    std::unordered_map<std::string, BlobIndex> lookups;
};

Unique<PackReader> PackReader::fromFile(const std::string& path) {
    PackReaderImpl* reader = new PackReaderImpl;

    int fd = open(path.c_str(), O_RDONLY);
    if (fd == -1) {
        delete reader;
        return Unique<PackReader>();
    }

    struct stat st;
    fstat(fd, &st);

    if (st.st_size == 0) {
        delete reader;
        return Unique<PackReader>();
    }

    reader->map = reinterpret_cast<char*>(mmap(nullptr, st.st_size, PROT_READ, MAP_SHARED, fd, 0));
    reader->mapLen = st.st_size;

    if (reader->map == MAP_FAILED) {
        delete reader;
        return Unique<PackReader>();
    }

    size_t offset = 0;

    reader->header = reinterpret_cast<HeaderBlock*>(reader->map + offset);
    offset += sizeof(*reader->header);

    if (memcmp(reader->header->magic,
               PACK_MAGIC,
               sizeof(reader->header->magic)) != 0) {
        munmap(reader->map, reader->mapLen);
        delete reader;
        return Unique<PackReader>();
    }

    if (reader->header->version != PACK_VERSION) {
        munmap(reader->map, reader->mapLen);
        delete reader;
        return Unique<PackReader>();
    }

    uint64_t blobCount = reader->header->blobCount;

    reader->pathOffsets = reinterpret_cast<PathOffset*>(reader->map + offset);
    offset += (blobCount + 1) * sizeof(PathOffset);

    reader->paths = reader->map + offset;
    offset += reader->header->pathsBlockSize;

    reader->metadatas = reinterpret_cast<BlobMetadata*>(reader->map + offset);
    offset += blobCount * sizeof(BlobMetadata);

    reader->dataOffsets = reinterpret_cast<uint64_t*>(reader->map + offset);
    offset += blobCount * sizeof(uint64_t);

    return Unique<PackReader>(reader);
}

PackReaderImpl::~PackReaderImpl() {
    munmap(map, mapLen);
}

PackReader::BlobIndex PackReaderImpl::size() const {
    return header->blobCount;
}

PackReader::BlobIndex PackReaderImpl::findIndex(const std::string& path) const {
    if (!lookupsConstructed) {
        lookupsConstructed = true;
        constructLookups();
    }

    auto it = lookups.find(path);
    if (it == lookups.end()) {
        return BLOB_NOT_FOUND;
    } else {
        return it->second;
    }
}

std::string PackReaderImpl::getBlobPath(PackReader::BlobIndex index) const {
    uint64_t begin = pathOffsets[index];
    uint64_t end = pathOffsets[index + 1];
    return std::string(paths + begin, paths + end);
}

uint64_t PackReaderImpl::getBlobSize(PackReader::BlobIndex index) const {
    return metadatas[index].uncompressedSize;
}

void* PackReaderImpl::getBlobData(PackReader::BlobIndex index) {
    uint64_t offset = dataOffsets[index];
    return map + offset;
}

vector<void*> PackReaderImpl::getBlobDatas(vector<BlobIndex> indicies) {
    vector<void*> datas;

    for (BlobIndex i : indicies) {
        datas.push_back(map + dataOffsets[i]);
    }

    return datas;
}

void PackReaderImpl::constructLookups(PackReaderImpl* reader) {
    for (PackReader::BlobIndex i = 0; i < blobCount; i++) {
        uint64_t pathBegin = pathOffsets[i];
        uint64_t pathEnd = pathOffsets[i + 1];
        std::string blobPath(paths + pathBegin, paths + pathEnd);
        lookups[blobPath] = i;
    }
}


class PackWriterImpl : public PackWriter {
 public:
    bool writeToFile(const std::string& path);

    void addBlob(std::string path, uint64_t size, const void* data);

 private:
    vector<Blob> blobs;
    bool sorted = true;
};

Unique<PackWriter> PackWriter::make() {
    return Unique<PackWriter>(new PackWriterImpl);
}

bool PackWriterImpl::writeToFile(const std::string &path) {
    size_t blobCount = blobs.size();

    // Sort blobs by size (smallest first).
    if (!sorted) {
        sorted = true;
        std::sort(blobs.begin(), blobs.end());
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
    std::string pathsBlock;
    vector<BlobMetadata> metadatasBlock;
    vector<uint64_t> dataOffsetsBlock;
    vector<iovec> ios;

    pathOffsetsBlock.reserve(blobCount + 1);
    pathsBlock.reserve(pathsBlockSize);
    metadatasBlock.reserve(blobCount);
    ios.reserve(5 + blobCount);

    PathOffset pathOffset = 0;
    for (auto& blob : blobs) {
        pathOffsetsBlock.push_back(pathOffset);
        pathOffset += blob.path.size();
    }
    pathOffsetsBlock.push_back(pathOffset);

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
