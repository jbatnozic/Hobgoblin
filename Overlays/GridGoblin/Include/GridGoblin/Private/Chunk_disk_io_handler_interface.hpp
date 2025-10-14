// Copyright 2024 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#pragma once

#include <GridGoblin/Model/Building_block.hpp>
#include <GridGoblin/Model/Chunk.hpp>
#include <GridGoblin/Model/Chunk_id.hpp>

#include <optional>

namespace jbatnozic {
namespace gridgoblin {

// Forward declare
class Binder;

namespace detail {

class ChunkDiskIoHandlerInterface {
public:
    virtual ~ChunkDiskIoHandlerInterface() = default;

    virtual void setBinder(Binder* aBinder) = 0;

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
