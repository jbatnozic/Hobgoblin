// Copyright 2024 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#include <GridGoblin/Private/Chunk_disk_io_handler_default.hpp>
#include <GridGoblin/World/Binder.hpp>

#include <Hobgoblin/Format.hpp>
#include <Hobgoblin/HGExcept.hpp>
#include <Hobgoblin/Utility/File_io.hpp>

#include <GridGoblin/Private/Model_conversions.hpp>

#include <fstream>

namespace jbatnozic {
namespace gridgoblin {
namespace detail {

namespace {
const std::filesystem::path CHUNKS_FOLDER = "DCIO_CHUNKS";
} // namespace

DefaultChunkDiskIoHandler::DefaultChunkDiskIoHandler(const WorldConfig& aConfig)
    : _basePath{aConfig.chunkDirectoryPath} //
{
    if (const auto path = _basePath / CHUNKS_FOLDER; !std::filesystem::exists(path)) {
        std::filesystem::create_directory(path);
    }
    _reusableConversionBuffers = NewReusableConversionBuffers();
}

DefaultChunkDiskIoHandler::~DefaultChunkDiskIoHandler() {
    DeleteReusableConversionBuffers(_reusableConversionBuffers);
}

void DefaultChunkDiskIoHandler::setBinder(Binder* aBinder) {
    _binder = aBinder;
}

std::optional<Chunk> DefaultChunkDiskIoHandler::loadChunkFromRuntimeCache(ChunkId aChunkId) {
    // TODO: make runtime cache a separate thing
    return loadChunkFromPersistentCache(aChunkId);
}

void DefaultChunkDiskIoHandler::storeChunkInRuntimeCache(const Chunk& aChunk, ChunkId aChunkId) {
    storeChunkInPersistentCache(aChunk, aChunkId);
}

std::optional<Chunk> DefaultChunkDiskIoHandler::loadChunkFromPersistentCache(ChunkId aChunkId) {
    const auto path = _buildPathToChunk(aChunkId);
    if (!std::filesystem::exists(path)) {
        return std::nullopt;
    }

    auto bytes = hg::util::SlurpFileBytes(path);

    auto chunkExtensionFactory = [this, aChunkId](const Chunk& aChunk) {
        HG_ASSERT(_binder != nullptr);
        auto extension = _binder->createChunkExtension();
        if (extension != nullptr) {
            extension->init(aChunkId, aChunk);
        }
        return extension;
    };

    return JsonStringToChunk(std::move(bytes), chunkExtensionFactory, _reusableConversionBuffers);
}

void DefaultChunkDiskIoHandler::storeChunkInPersistentCache(const Chunk& aChunk, ChunkId aChunkId) {
    const auto path = _buildPathToChunk(aChunkId);

    auto str = ChunkToJsonString(aChunk, _reusableConversionBuffers);

    std::ofstream file{path, std::ios::out | std::ios::binary | std::ios::trunc};
    HG_HARD_ASSERT(file.is_open() && file.good());
    file << str;
}

std::filesystem::path DefaultChunkDiskIoHandler::_buildPathToChunk(ChunkId aChunkId) const {
    return _basePath / CHUNKS_FOLDER / fmt::format(FMT_STRING("chunk_{}_{}"), aChunkId.x, aChunkId.y);
}

void DefaultChunkDiskIoHandler::dumpRuntimeCache() {}

} // namespace detail
} // namespace gridgoblin
} // namespace jbatnozic
