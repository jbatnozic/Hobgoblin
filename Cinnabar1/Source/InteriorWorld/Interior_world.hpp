// Copyright 2026 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#pragma once

#include <Engine.hpp>

#include <GridGoblin/World/World.hpp>

// #include <InteriorWorld/Graph_of_attachables.hpp>

#include <memory>

namespace cinnabar {

class InteriorWorld /* a.k.a. Inland Empire :D */ : private jbatnozic::gridgoblin::Binder {
public:
    constexpr static hg::math::Vector2d CENTER_OFFSET = {1536.0, 3072.0};

    InteriorWorld();

private:
    jbatnozic::gridgoblin::World _world;

    // TODO: vector - list of associated player IDs

    // Inherited from GridGoblin Binder

    void didPrepareChunk(jbatnozic::gridgoblin::ChunkId aChunkId) override;

    void willIntegrateNewChunk(
        jbatnozic::gridgoblin::ChunkId                      aId,
        jbatnozic::gridgoblin::Chunk&                       aChunk,
        const jbatnozic::gridgoblin::ChunkMemoryLayoutInfo& aChunkMemLayout) override;

    void willIntegrateLoadedChunk(
        jbatnozic::gridgoblin::ChunkId                      aId,
        jbatnozic::gridgoblin::Chunk&                       aChunk,
        const jbatnozic::gridgoblin::ChunkMemoryLayoutInfo& aChunkMemLayout) override;

    void didIntegrateChunk(jbatnozic::gridgoblin::ChunkId                      aId,
                           const jbatnozic::gridgoblin::Chunk&                 aChunk,
                           const jbatnozic::gridgoblin::ChunkMemoryLayoutInfo& aChunkMemLayout) override;

    void willSeparateChunk(jbatnozic::gridgoblin::ChunkId                      aId,
                           const jbatnozic::gridgoblin::Chunk&                 aChunk,
                           const jbatnozic::gridgoblin::ChunkMemoryLayoutInfo& aChunkMemLayout) override;

    void didSeparateChunk(jbatnozic::gridgoblin::ChunkId                      aId,
                          jbatnozic::gridgoblin::Chunk&                       aChunk,
                          const jbatnozic::gridgoblin::ChunkMemoryLayoutInfo& aChunkMemLayout) override;

    void didEditCells(const jbatnozic::gridgoblin::CellEditInfos& aCellEditInfos) override;

    std::unique_ptr<jbatnozic::gridgoblin::ChunkExtensionInterface> createChunkExtension() override;
};

} // namespace cinnabar
