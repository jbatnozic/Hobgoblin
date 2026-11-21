// Copyright 2026 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#pragma once

#include <Engine.hpp>
#include <CBR/InteriorWorld/Internal_runtime.hpp>
#include <CBR/Ship/Onboard_player_info.hpp>

#include <Hobgoblin/Utility/Stream_input.hpp>
#include <Hobgoblin/Utility/Stream_output.hpp>
#include <GridGoblin/World/World.hpp>

#include <memory>
#include <optional>

namespace cinnabar {

class InteriorWorld /* a.k.a. Inland Empire :D */ : private jbatnozic::gridgoblin::Binder {
public:
    constexpr static double             CELL_RESOLUTION = 48.0;
    constexpr static hg::PZInteger      CELL_COUNT_X    = 1024;
    constexpr static hg::PZInteger      CELL_COUNT_Y    = 2048;
    constexpr static hg::math::Vector2d CENTER_OFFSET   = {1536.0, 3072.0};

    enum class Mode {
        MASTER,
        DUMMY
    };

    InteriorWorld(Mode aMode, spe::GameContext& aContext);

    const OnboardPlayerInfo& getOnboardPlayerInfo() const;

    ///////////////////////////////////////////////////////////////////////////
    // MARK: CYCLE METHODS                                                   //
    ///////////////////////////////////////////////////////////////////////////

    //! \brief The active controller of this object should call this during the
    //!        `_beginUpdate()` event so the IW can prepare for the update step.
    void onBeginUpdate();

    //! \brief The active controller of this object should call this during the
    //!        `_endUpdate()` event so the IW can clean up after the update step.
    void onEndUpdate();

    ///////////////////////////////////////////////////////////////////////////
    // MARK: WORLD METHODS                                                   //
    ///////////////////////////////////////////////////////////////////////////

    //! \brief return a read-only reference to the underlying GridGoblin world instance.
    const jbatnozic::gridgoblin::World& getUnderlying() const;

    template <class taCallable>
    void editWorld(taCallable&& aCallable);

    ///////////////////////////////////////////////////////////////////////////
    // MARK: SYNCHRONIZATION METHODS                                         //
    ///////////////////////////////////////////////////////////////////////////

    //! \brief Write the state off all cells of all available chunks of the
    //!        underlying GridGoblin world into a stream.
    void writeWholeCellState(hg::util::OutputStream& aOStream) const;

    //! \brief Set the state of all cells of the underlying GridGoblin world according
    //!        to the world state representation read from the given stream.
    void readWholeCellState(hg::util::InputStream& aIStream);

    //! \returns whether anything was written to the stream or not
    bool writeCellStateDiff(hg::util::OutputStream& aOStream) const;

    void readAndApplyCellStateDiff(hg::util::InputStream& aIStream);

    //! \brief Force every Synchronized Object held by the IW's internal runtime to immediately
    //!        compose messages to sync its CREATE and UPDATE events to the given client.
    void doFullSyncOfCreateAndUpdateOfEntireInternalRuntimeForClient(hg::PZInteger aClientIndex);

private:
    Mode _mode;

    jbatnozic::gridgoblin::World         _world;
    jbatnozic::gridgoblin::CellEditInfos _cellEditInfos;

    InternalRuntime _internalRuntime;

    // TODO: vector - list of associated player IDs

    bool _getPreferredWorldEditJournalingFlag() const;

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

template <class taCallable>
void InteriorWorld::editWorld(taCallable&& aCallable) {
    _world.edit(*_world.getPermissionToEdit(), std::forward<taCallable>(aCallable));
}

} // namespace cinnabar
