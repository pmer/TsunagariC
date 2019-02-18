/*************************************
** Tsunagari Tile Engine            **
** pack-reader.cpp                  **
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

#include "pack/pack-reader.h"

#include <stdint.h>
#include <string.h>

#include <string>
#include <unordered_map>

#include "os/os.h"
#include "util/move.h"
#include "util/optional.h"

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

class PackReaderImpl : public PackReader {
 public:
    BlobIndex size() const;

    BlobIndex findIndex(const std::string& path);

    std::string getBlobPath(BlobIndex index) const;
    uint64_t getBlobSize(BlobIndex index) const;
    void* getBlobData(BlobIndex index);

    vector<void*> getBlobDatas(vector<BlobIndex> indicies);

 public:
    void constructLookups();

 public:
    MappedFile file;

    // Pointers into `file`.
    const HeaderBlock* header;
    const PathOffset* pathOffsets;
    const char* paths;
    const BlobMetadata* metadatas;
    const uint64_t* dataOffsets;

    bool lookupsConstructed = false;
    std::unordered_map<std::string, BlobIndex> lookups;
};

Unique<PackReader> PackReader::fromFile(const std::string& path) {
    Optional<MappedFile> maybeFile = MappedFile::fromPath(path);
    if (!maybeFile) {
        return Unique<PackReader>();
    }

    MappedFile file = move_(*maybeFile);

    size_t offset = 0;

    HeaderBlock* header = file.at<HeaderBlock*>(offset);
    offset += sizeof(*header);

    if (memcmp(header->magic, PACK_MAGIC, sizeof(header->magic)) != 0) {
        return Unique<PackReader>();
    }

    if (header->version != PACK_VERSION) {
        return Unique<PackReader>();
    }

    PackReaderImpl* reader = new PackReaderImpl;
    reader->file = move_(file);
    reader->header = header;

    uint64_t blobCount = header->blobCount;

    reader->pathOffsets = reader->file.at<PathOffset*>(offset);
    offset += (blobCount + 1) * sizeof(PathOffset);

    reader->paths = reader->file.at<char*>(offset);
    offset += header->pathsBlockSize;

    reader->metadatas = reader->file.at<BlobMetadata*>(offset);
    offset += blobCount * sizeof(BlobMetadata);

    reader->dataOffsets = reader->file.at<uint64_t*>(offset);
    offset += blobCount * sizeof(uint64_t);

    return Unique<PackReader>(reader);
}

PackReader::BlobIndex PackReaderImpl::size() const {
    return header->blobCount;
}

PackReader::BlobIndex PackReaderImpl::findIndex(const std::string& path) {
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
    return file.at<void*>(dataOffsets[index]);
}

vector<void*> PackReaderImpl::getBlobDatas(vector<BlobIndex> indicies) {
    vector<void*> datas;

    for (BlobIndex i : indicies) {
        datas.push_back(file.at<void*>(dataOffsets[i]));
    }

    return datas;
}

void PackReaderImpl::constructLookups() {
    for (PackReader::BlobIndex i = 0; i < header->blobCount; i++) {
        uint64_t pathBegin = pathOffsets[i];
        uint64_t pathEnd = pathOffsets[i + 1];
        std::string blobPath(paths + pathBegin, paths + pathEnd);
        lookups[blobPath] = i;
    }
}
