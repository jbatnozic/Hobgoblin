// Copyright 2024 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#include <GridGoblin/Private/Disk_io_handler_default.hpp>
#include <GridGoblin/World/Binder.hpp>

#include <Hobgoblin/Format.hpp>
#include <Hobgoblin/HGExcept.hpp>
#include <Hobgoblin/Logging.hpp>
#include <Hobgoblin/Utility/File_io.hpp>

#include <GridGoblin/Private/Model_conversions.hpp>

#include <fstream>

namespace jbatnozic {
namespace gridgoblin {
namespace detail {

#define LOG_ID "GridGoblin"

namespace fs = std::filesystem;

namespace {
const fs::path CHUNKS_DIR              = "dcio_chunks";
const fs::path RUNTIME_CACHE_SUBDIR    = "runtime_cache";
const fs::path PERSISTENT_CACHE_SUBDIR = "persistent_cache";
} // namespace

DefaultDiskIoHandler::DefaultDiskIoHandler(const StorageConfig& aConfig)
    : _storageDirectory{aConfig.storageDirectory} //
{
    _reusableConversionBuffers = NewReusableConversionBuffers();
}

DefaultDiskIoHandler::~DefaultDiskIoHandler() {
    DeleteReusableConversionBuffers(_reusableConversionBuffers);
}

void DefaultDiskIoHandler::setBinder(Binder* aBinder) {
    _binder = aBinder;
}

void DefaultDiskIoHandler::checkOrInitWorldFiles(const ContentsConfig& aContentsConfig,
                                                 const StorageConfig*  aStorageConfig) {
    HG_VALIDATE_ARGUMENT(aStorageConfig != nullptr);

    const auto& storageDir = aStorageConfig->storageDirectory;

    // Check/create to-level storage directory
    if (!fs::exists(storageDir)) {
        if (aStorageConfig->allowCreateNew) {
            fs::create_directories(storageDir);

            // init new
            ContentsConfig::saveToStorageDirectory(aContentsConfig, storageDir);
            fs::create_directory(storageDir / RUNTIME_CACHE_SUBDIR);
            fs::create_directory(storageDir / PERSISTENT_CACHE_SUBDIR);

            return;
        } else {
            HG_THROW_TRACED(hobgoblin::TracedRuntimeError,
                            0,
                            "Cannot initialize world storage at '{}' - the directory doesn't exist and "
                            "creating it was explicitly forbidden.",
                            storageDir.string());
        }
    } else if (!fs::is_directory(storageDir)) {
        HG_THROW_TRACED(
            hobgoblin::TracedRuntimeError,
            0,
            "Cannot initialize world storage at '{}' - the path exists but it's not a directory.",
            storageDir.string());
    }

    // Check/create contents config file
    if (!fs::exists(storageDir / CONTENTS_CONFIG_FILE_NAME)) {
        if (aStorageConfig->allowCreateNew) {
            ContentsConfig::saveToStorageDirectory(aContentsConfig, storageDir);
        } else {
            HG_THROW_TRACED(hobgoblin::TracedRuntimeError,
                            0,
                            "Cannot initialize world storage at '{}' - the file '{}' doesn't exist and "
                            "creating it was explicitly forbidden.",
                            storageDir.string(),
                            CONTENTS_CONFIG_FILE_NAME);
        }
    } else {
        const auto existingConfig = ContentsConfig::loadFromStorageDirectory(storageDir);
        if (existingConfig != aContentsConfig) {
            if (aStorageConfig->allowOverwriteConfig) {
                ContentsConfig::saveToStorageDirectory(aContentsConfig, storageDir);
            } else {
                HG_THROW_TRACED(
                    hobgoblin::TracedRuntimeError,
                    0,
                    "Cannot initialize world storage at '{}' - the stored contents configuration is "
                    "different from the requested one and overwriting it was explicitly forbidden.",
                    storageDir.string());
            }
        }
    }

    // Check/create cache directories
    {
        const auto chunksDir           = storageDir / CHUNKS_DIR;
        const auto runtimeCachePath    = chunksDir / RUNTIME_CACHE_SUBDIR;
        const auto persistentCachePath = chunksDir / PERSISTENT_CACHE_SUBDIR;
        const decltype(chunksDir)* paths[] = {
            &chunksDir, &runtimeCachePath, &persistentCachePath
        };
        for (const auto p : paths) {
            if (!fs::exists(*p)) {
                fs::create_directory(*p);
            } else if (!fs::is_directory(*p)) {
                HG_THROW_TRACED(hobgoblin::TracedRuntimeError,
                                0,
                                "Cannot initialize world storage at '{}' - the path '{}' exists but "
                                "it's not a directory.",
                                storageDir.string(),
                                p->string());
            }
        }

        // If runtime cache is not empty, empty it and warn
        if (!fs::is_empty(runtimeCachePath)) {
            HG_LOG_WARN(LOG_ID,
                        "Found non-empty runtime cache directory at '{}' - emptying...",
                        runtimeCachePath.string());
            for (const auto& entry : fs::directory_iterator(runtimeCachePath)) {
                std::filesystem::remove_all(entry.path());
            }
        }
    }
}

std::optional<Chunk> DefaultDiskIoHandler::loadChunkFromRuntimeCache(
    ChunkId                      aChunkId,
    const ChunkMemoryLayoutInfo& aChunkMemoryLayout) //
{
    return _loadChunkImpl(aChunkId, aChunkMemoryLayout, RUNTIME_CACHE_SUBDIR);
}

void DefaultDiskIoHandler::storeChunkInRuntimeCache(const Chunk&                 aChunk,
                                                    ChunkId                      aChunkId,
                                                    BuildingBlockMask            aBuildingBlocks,
                                                    const ChunkMemoryLayoutInfo& aChunkMemoryLayout) //
{
    _storeChunkImpl(aChunk, aChunkId, aBuildingBlocks, aChunkMemoryLayout, RUNTIME_CACHE_SUBDIR);
}

std::optional<Chunk> DefaultDiskIoHandler::loadChunkFromPersistentCache(
    ChunkId                      aChunkId,
    const ChunkMemoryLayoutInfo& aChunkMemoryLayout) //
{
    return _loadChunkImpl(aChunkId, aChunkMemoryLayout, PERSISTENT_CACHE_SUBDIR);
}

void DefaultDiskIoHandler::storeChunkInPersistentCache(
    const Chunk&                 aChunk,
    ChunkId                      aChunkId,
    BuildingBlockMask            aBuildingBlocks,
    const ChunkMemoryLayoutInfo& aChunkMemoryLayout) //
{
    _storeChunkImpl(aChunk, aChunkId, aBuildingBlocks, aChunkMemoryLayout, PERSISTENT_CACHE_SUBDIR);
}

void DefaultDiskIoHandler::dumpRuntimeCache() {
    // TODO
}

fs::path DefaultDiskIoHandler::_buildPathToChunk(ChunkId                      aChunkId,
                                                 const std::filesystem::path& aCacheSubdir) const {
    return _storageDirectory / CHUNKS_DIR / aCacheSubdir /
           fmt::format(FMT_STRING("chunk_{}_{}"), aChunkId.x, aChunkId.y);
}

std::optional<Chunk> DefaultDiskIoHandler::_loadChunkImpl(
    ChunkId                      aChunkId,
    const ChunkMemoryLayoutInfo& aChunkMemoryLayout,
    const std::filesystem::path& aCacheSubdir) const //
{
    const auto path = _buildPathToChunk(aChunkId, aCacheSubdir);
    if (!fs::exists(path)) {
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

    return JsonStringToChunk(std::move(bytes),
                             aChunkMemoryLayout,
                             chunkExtensionFactory,
                             _reusableConversionBuffers);
}

void DefaultDiskIoHandler::_storeChunkImpl(const Chunk&                 aChunk,
                                           ChunkId                      aChunkId,
                                           BuildingBlockMask            aBuildingBlocks,
                                           const ChunkMemoryLayoutInfo& aChunkMemoryLayout,
                                           const std::filesystem::path& aCacheSubdir) const {
    const auto path = _buildPathToChunk(aChunkId, aCacheSubdir);

    auto str =
        ChunkToJsonString(aChunk, aBuildingBlocks, aChunkMemoryLayout, _reusableConversionBuffers);

    std::ofstream file{path, std::ios::out | std::ios::binary | std::ios::trunc};
    HG_HARD_ASSERT(file.is_open() && file.good());
    file << str;
}

} // namespace detail
} // namespace gridgoblin
} // namespace jbatnozic
