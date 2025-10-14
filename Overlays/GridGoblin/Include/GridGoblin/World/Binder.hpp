// Copyright 2024 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#pragma once

#include <GridGoblin/Model/Cell.hpp>
#include <GridGoblin/Model/Chunk.hpp>
#include <GridGoblin/Model/Chunk_extension.hpp>
#include <GridGoblin/Model/Chunk_id.hpp>

#include <GridGoblin/Private/Chunk_holder.hpp>

#include <Hobgoblin/Math/Vector.hpp>

#include <memory>
#include <unordered_map>

namespace jbatnozic {
namespace gridgoblin {

namespace gh = ::jbatnozic::hobgoblin;

//! Contains information about a cell that was edited.
//! \note it does NOT contain the ID of the edited cell, but \see `CellEditInfos`.
struct CellEditInfo {
    //! Mask of all the building blocks that were edited in the indicated cell.
    BuildingBlockMask bbMask;
};

//! A container used to inform a listener about changes to cells of a world.
//! It maps IDs of edited cells (represented as `hg::math::Vector2pz` - providing absolute X and Y
//! coordinates of cells in the world) to associated `CellEditInfo` objects.
using CellEditInfos = std::unordered_map<hg::math::Vector2pz, CellEditInfo>;

//! This class contains a number of callbacks which can be overriden to 'bind'
//! a GridGoblin World to its enclosing application and help them interoperate better.
class Binder {
public:
    //! Virtual destructor.
    virtual ~Binder() = default;

    //! Called when a previously requested chunk (either requested explicitly via an `ActiveArea`, or
    //! for on-demand loading via a getter) is ready to be integrated into the world. This means that
    //! on the next `World::update()` call, this chunk will become available.
    //!
    //! \note in general the purpose of this callback is to let the controlling code know that it needs
    //!       to call `World::update()` because there are chunks waiting to be integrated. The only
    //!       other intended use for it is testing.
    //!
    //! \warning this method can (and usually will be) called from a background thread, AT ANY TIME! Be
    //!          very careful what you put in it - anything outside of the suggestions noted above is
    //!          very much not recommended, and editing the world in any way doubly so!
    virtual void didPrepareChunk(ChunkId aChunkId) {}

    //! Called when a new chunk is about to be integrated into the World. By 'new' we mean that access
    //! to this chunk was requested but it could not be found in the cache, so a new chunk was
    //! default-constructed (meaning that all of the fields of all the cells in this chunk will have
    //! random, basically uninitialized values).
    //!
    //! Unless all of the chunks in the game are premade using an editor, this callback is the right
    //! place to generate the contents of the new chunk.
    //!
    //! \param aId ID of the new chunk.
    //! \param aChunk non-const reference to the new chunk.
    //! \param aChunkMemLayout the chunk's memory layout descriptor (needed when calling the chunk's
    //!                        methods).
    //!
    //! \note the default implementation of this method will just set all fields to zero, which is
    //!       almost certainly not what you want.
    //!
    //! \warning this callback is called BEFORE the chunk becomes available through the World instance
    //!          (and its methods like `getCellDataAt` etc.). For the same reason, the `openness` and
    //!          `obFlags` values of the cells in this chunks will not have been calculated yet when
    //!          this callback is called.
    //!
    //! \warning DO NOT make the chunk empty in this callback (by reassigning it or otherwise)!
    //!
    //! \note this method can only be called in response to 2 different situations:
    //!       - the World is being updated (`World::update()` called), and the chunk was requested by a
    //!         change in one of the active areas prior;
    //!       - the chunk was created on-demand, triggered by one of the World's getters or setters.
    //!       In all three cases, `onChunkCreated` is called from the same thread which triggered any of
    //!       the above conditions.
    virtual void willIntegrateNewChunk(ChunkId                      aId,
                                       Chunk&                       aChunk,
                                       const ChunkMemoryLayoutInfo& aChunkMemLayout);

    //! Called when a pre-existing chunk is about to be integrated into the World. By 'pre-existing' we
    //! mean that this chunk was used before, but has been unloaded since, so now that access to it
    //! is needed again, it has been loaded from the cache once more.
    //!
    //! This callback gives the chance to edit the cells or the extension before it's integrated with
    //! the rest of the world.
    //!
    //! \param aId ID of the loaded chunk.
    //! \param aChunk non-const reference to the loaded chunk.
    //! \param aChunkMemLayout the chunk's memory layout descriptor (needed when calling the chunk's
    //!                        methods).
    //!
    //! \warning this callback is called BEFORE the chunk becomes available through the World instance
    //!          (and its methods like `getCellDataAt` etc.). For the same reason, the `openness` and
    //!          `obFlags` values of the cells in this chunks will not have been calculated yet when
    //!          this callback is called.
    //!
    //! \warning DO NOT make the chunk empty in this callback (by reassigning it or otherwise)!
    //!
    //! \note this method can only be called in response to 3 different situations:
    //!       - the World is being updated (`World::update()` called), and the chunk was requested by a
    //!         change in one of the active areas prior;
    //!       - one of the active areas is changed (`ActiveArea::set*()` called) - sometimes, rarely, the
    //!         chunk is immediately available;
    //!       - the chunk was created on-demand, triggered by one of the World's getters or setters.
    //!       In all three cases, `onChunkCreated` is called from the same thread which triggered any of
    //!       the above conditions.
    virtual void willIntegrateLoadedChunk(ChunkId                      aId,
                                          Chunk&                       aChunk,
                                          const ChunkMemoryLayoutInfo& aChunkMemLayout) {}

    //! After `willIntegrateNewChunk` or `willIntegrateLoadedChunk` is called, the chunk is integrated
    //! into the world (becoming available for methods like `getCellDataAt` and others), the `openness`
    //! and `obFlags` values of spatial infos of its cells are calculated, and then `didIntegrateChunk`
    //! is called, from the same thread that called the `willIntegrate*` callback.
    //!
    //! \param aId ID of the integrated chunk.
    //! \param aChunk reference to the integrated chunk, this time read-only.
    //! \param aChunkMemLayout the chunk's memory layout descriptor (needed when calling the chunk's
    //!                        methods).
    //!
    //! \note the intended way to use this method is as follows:
    //!       - once a chunk is about to be de-integrated and unloaded, signalled by the
    //!         `willSeparateChunk` callback, all the objects contained within this chunk that are not
    //!         cells can be serialized or otherwise saved in the chunk's extension.
    //!       - `didIntegrateChunk` is supposed to do the opposite - restore all of these objects after
    //!         the chunk itself is restored.
    virtual void didIntegrateChunk(ChunkId                      aId,
                                   const Chunk&                 aChunk,
                                   const ChunkMemoryLayoutInfo& aChunkMemLayout) {}

    //! Called when a chunk is about to be de-integrated from the world and unloaded.
    //! However, at the time of calling this, it's still accessible through the world instance.
    //!
    //! \param aId ID of the chunk about to be de-integrated.
    //! \param aChunk read-only reference to the chunk about to be de-integrated.
    //! \param aChunkMemLayout the chunk's memory layout descriptor (needed when calling the chunk's
    //!                        methods).
    //!
    //! \note the intended way to use this method is as follows:
    //!       - once a chunk is about to be de-integrated and unloaded, signalled by this callback,
    //!         all the objects contained within this chunk that are not cells can be serialized or
    //!         otherwise saved in the chunk's extension.
    //!       - `didIntegrateChunk` is supposed to do the opposite - restore all of these objects after
    //!         the chunk itself is restored.
    virtual void willSeparateChunk(ChunkId                      aId,
                                   const Chunk&                 aChunk,
                                   const ChunkMemoryLayoutInfo& aChunkMemLayout) {}

    //! After `willSeparateChunk` is called, the chunk is de-integrated from world (becoming unavailable
    //! for methods like `getCellDataAt` and others), and then this method is called. In it, you can
    //! change any of the chunk's values before it's serialized and saved in the cache for later use.
    //!
    //! \param aId ID of the de-integrated chunk.
    //! \param aChunk non-const reference to the de-integrated chunk.
    //! \param aChunkMemLayout the chunk's memory layout descriptor (needed when calling the chunk's
    //!                        methods).
    //!
    //! \warning this callback is called AFTER the chunk becomes unavailable through the World instance
    //!          (and its methods like `getCellDataAt` etc.).
    virtual void didSeparateChunk(ChunkId                      aId,
                                  Chunk&                       aChunk,
                                  const ChunkMemoryLayoutInfo& aChunkMemLayout) {}

    //! Called at the very end of a `World::edit(...)` call, just before it returns.
    //! Provides a list of cells which were edited, and a mask for each cell indicating which
    //! of its building block structs were edited.
    virtual void didEditCells(const CellEditInfos& aCellEditInfos) {}

    //! Called by the world instance during chunk creation when an extension is needed to
    //! attach to a chunk.
    virtual std::unique_ptr<ChunkExtensionInterface> createChunkExtension() {
        return nullptr;
    }
};

inline void Binder::willIntegrateNewChunk(ChunkId                      aId,
                                          Chunk&                       aChunk,
                                          const ChunkMemoryLayoutInfo& aChunkMemLayout) {
    (void)aId;
    aChunk.zeroOut(aChunkMemLayout);
}

} // namespace gridgoblin
} // namespace jbatnozic
