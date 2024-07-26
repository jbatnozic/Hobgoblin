#pragma once

#include <GridWorld/Model/Chunk.hpp>
#include <GridWorld/Model/Chunk_id.hpp>

#include <optional>

namespace gridworld {
namespace detail {

class ChunkDiskIoHandlerInterface {
public:
    virtual ~ChunkDiskIoHandlerInterface() = default;

    virtual std::optional<Chunk> loadChunkFromRuntimeCache(ChunkId aChunkId) = 0;

    virtual void storeChunkInRuntimeCache(const Chunk& aChunk, ChunkId aChunkId) = 0;

    virtual std::optional<Chunk> loadChunkFromPersistentCache(ChunkId aChunkId) = 0;

    virtual void storeChunkInPersistentCache(const Chunk& aChunk, ChunkId aChunkId) = 0;
};

} // namespace detail
} // namespace gridworld