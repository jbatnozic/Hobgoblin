// Copyright 2024 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#pragma once

#include <GridGoblin/Model/Building_block.hpp>
#include <GridGoblin/Model/Chunk.hpp>
#include <GridGoblin/Model/Chunk_id.hpp>
#include <GridGoblin/World/World_config.hpp>

#include <optional>

namespace jbatnozic {
namespace gridgoblin {

// Forward declare
class Binder;

namespace detail {

class DiskIoHandlerInterface {
public:
    virtual ~DiskIoHandlerInterface() = default;

    virtual void setBinder(Binder* aBinder) = 0;

    virtual void checkOrInitWorldFiles(const ContentsConfig& aContentsConfig,
                                       const StorageConfig*  aStorageConfig) = 0;

    virtual std::optional<Chunk> loadChunkFromRuntimeCache(
        ChunkId                      aChunkId,
        const ChunkMemoryLayoutInfo& aChunkMemoryLayout) = 0;

    virtual void storeChunkInRuntimeCache(const Chunk&                 aChunk,
                                          ChunkId                      aChunkId,
                                          BuildingBlockMask            aBuildingBlocks,
                                          const ChunkMemoryLayoutInfo& aChunkMemoryLayout) = 0;

    virtual std::optional<Chunk> loadChunkFromPersistentCache(
        ChunkId                      aChunkId,
        const ChunkMemoryLayoutInfo& aChunkMemoryLayout) = 0;

    virtual void storeChunkInPersistentCache(const Chunk&                 aChunk,
                                             ChunkId                      aChunkId,
                                             BuildingBlockMask            aBuildingBlocks,
                                             const ChunkMemoryLayoutInfo& aChunkMemoryLayout) = 0;

    virtual void dumpRuntimeCache() = 0;
};

} // namespace detail
} // namespace gridgoblin
} // namespace jbatnozic
