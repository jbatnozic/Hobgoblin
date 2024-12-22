// Copyright 2024 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#pragma once

#include <Griddy/Private/Chunk_disk_io_handler_interface.hpp>

#include <Griddy/Model/Chunk.hpp>
#include <Griddy/Model/Chunk_id.hpp>
#include <Griddy/World/World_config.hpp>

#include <filesystem>
#include <optional>

namespace jbatnozic {
namespace griddy {

// Forward declare
class Binder;

namespace detail {

class DefaultChunkDiskIoHandler : public ChunkDiskIoHandlerInterface {
public:
    DefaultChunkDiskIoHandler(const WorldConfig& aConfig);

    ~DefaultChunkDiskIoHandler() override;

    void setBinder(Binder* aBinder) override;

    std::optional<Chunk> loadChunkFromRuntimeCache(ChunkId aChunkId) override;

    void storeChunkInRuntimeCache(const Chunk& aChunk, ChunkId aChunkId) override;

    std::optional<Chunk> loadChunkFromPersistentCache(ChunkId aChunkId) override;

    void storeChunkInPersistentCache(const Chunk& aChunk, ChunkId aChunkId) override;

private:
    Binder* _binder = nullptr;

    std::filesystem::path _basePath;

    std::filesystem::path _buildPathToChunk(ChunkId aChunkId) const;
};

} // namespace detail
} // namespace griddy
}
