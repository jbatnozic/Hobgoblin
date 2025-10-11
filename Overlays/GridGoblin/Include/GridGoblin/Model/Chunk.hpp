// Copyright 2024 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#pragma once

#include <Hobgoblin/Math/Vector2.hpp>

#include <GridGoblin/Model/Cell.hpp>
#include <GridGoblin/Private/Chunk_impl.hpp>

#include <cassert>

namespace jbatnozic {
namespace gridgoblin {

namespace hg = jbatnozic::hobgoblin;

class ChunkExtensionInterface;

//! All the chunks of a single GridGoblin World must have the same memory layout, so the structure
//! describing this layout is saved only once, in the World instance, instead of being duplicated
//! in every chunk.
//! The downside of this is that when you're working with chunks directly, a lot of methods need a
//! reference to this structure passed to them - to get it, use `World::getChunkMemoryLayoutInfo()`.
//!
//! \note this is an opaque type. As a user of the library, you don't need to worry about the specific
//!       implementation of it - you'll just be dealing with references to it.
struct ChunkMemoryLayoutInfo;

class Chunk {
public:
    // clang-format off
    //! Default construction (constructs an empty chunk)
    //! \note if you're not a library maintainer and you're constructing chunks manually,
    //!       you're most likely doing something wrong.
    Chunk()                        = default;
    // No copying
    Chunk(const Chunk&)            = delete;
    Chunk& operator=(const Chunk&) = delete;
    // Moving is OK
    Chunk(Chunk&&)                 = default;
    Chunk& operator=(Chunk&&)      = default;
    // clang-format on

    //! Constructs a non-empty chunk according to the passed memory layout descriptor.
    //! \note if you're not a library maintainer and you're constructing chunks manually,
    //!       you're most likely doing something wrong.
    Chunk(const ChunkMemoryLayoutInfo& aMemLayout);

    //! Tell if the chunk is empty.
    //! \warning when a chunk is empty, all operations on it other than destroying it or
    //!          assigning a new value to it will produce Undefined Behaviour!
    bool isEmpty() const;

    ///////////////////////////////////////////////////////////////////////////
    // MARK: CELLS                                                           //
    ///////////////////////////////////////////////////////////////////////////

    //! \brief Retrieve cell data at a specified position within the chunk without bounds checking.
    //!
    //! This function accesses the data stored at the cell located at the given (`aX`, `aY`)
    //! coordinates, relative to the top-left of the chunk.
    //! \warning This function performs no bounds checking, so the caller must ensure the
    //!          coordinates are in range!
    //!
    //! The data is copied into preallocated objects from the `cell` namespace - the pointers
    //! `aPtrs...` must point to these preallocated objects.
    //! \warning passing any null pointers will lead to undefined behaviour!
    //!
    //! \param aMemLayout Memory layout descriptor for the chunk.
    //! \param aX X-coordinate of the target cell, relative to the top-left of the chunk.
    //! \param aY Y-coordinate of the target cell, relative to the top-left of the chunk.
    //! \param aPtrs One or more non-null pointers to objects from the `cell` namespace
    //!              where the retrieved data should be copied.
    //!
    //! Example of usage:
    //! \code
    //!     cell::CellKindId  cellKindId;
    //!     cell::SpatialInfo spatialInfo;
    //!     chunk.getCellDataAtUnchecked(memLayoutInfo, x, y, &cellKindId, &spatialInfo);
    //! \endcode
    template <class... taPtrs>
    void getCellDataAtUnchecked(const ChunkMemoryLayoutInfo& aMemLayout,
                                hg::PZInteger                aX,
                                hg::PZInteger                aY,
                                taPtrs&&... aPtrs) const;

    //! Same as the pther overload of `getCellDataAtUnchecked`,
    //! but here the coordinates are given as a vector.
    template <class... taPtrs>
    void getCellDataAtUnchecked(const ChunkMemoryLayoutInfo& aMemLayout,
                                hg::math::Vector2pz          aCell,
                                taPtrs&&... aPtrs) const;

    //! Convenience function to get the SpatialInfo specifically of the cell at `aCell`.
    //! \param aMemLayout Memory layout descriptor for the chunk.
    //! \param aCell X and Y coordinates of the target cell, relative to the top-left of the chunk.
    cell::SpatialInfo getSpatialInfoAtUnchecked(const ChunkMemoryLayoutInfo& aMemLayout,
                                                hg::math::Vector2pz          aCell) const;

    //! This function does the opposite of `getCellDataAtUnchecked` - the semantics of all the
    //! parameters are the same, but it copies the data from the objects pointed to by `aPtrs`
    //! into the chunk instead of the other way around.
    template <class... taPtrs>
    void setCellDataAtUnchecked(const ChunkMemoryLayoutInfo& aMemLayout,
                                hg::PZInteger                aX,
                                hg::PZInteger                aY,
                                taPtrs&&... aPtrs);

    //! Same as the other overload of `setCellDataAtUnchecked`,
    //! but here the coordinates are given as a vector.
    template <class... taPtrs>
    void setCellDataAtUnchecked(const ChunkMemoryLayoutInfo& aMemLayout,
                                hg::math::Vector2pz          aCell,
                                taPtrs&&... aPtrs);

    //! Set the values of all the fields of all the cells in this chunk to zero.
    //! \note the extension pointger remains unchanged.
    //! \throws PreconditionNotMetError is the chunk is empty.
    void zeroOut(const ChunkMemoryLayoutInfo& aMemLayout);

    //! Set all the cells in the chunk to the given value, unless the chunk is empty. Utility method.
    void setAll(const ChunkMemoryLayoutInfo& aMemLayout, const FatCell& aCell);

    ///////////////////////////////////////////////////////////////////////////
    // MARK: EXTENSIONS                                                      //
    ///////////////////////////////////////////////////////////////////////////

    //! Set a new extension for the chunk, destroying the old one (if any).
    void setExtension(std::unique_ptr<ChunkExtensionInterface> aChunkExtension);

    //! Return a mutable but non-owning pointer to the chunk's extension (if any; returns
    //! `nullptr` if no extension).
    ChunkExtensionInterface* getExtension() const;

    //! Detach the chunk's extension and return it owned by a `std::unique_ptr` (if any; returns
    //! `nullptr` if no extension).
    std::unique_ptr<ChunkExtensionInterface> releaseExtension();

private:
    detail::ChunkImpl _impl;

    // Getters

    void _getCellDataAtUnchecked(const ChunkMemoryLayoutInfo& aMemLayout,
                                 hg::PZInteger                aX,
                                 hg::PZInteger                aY,
                                 cell::CellKindId*            aCellKindId) const {
        assert(aCellKindId != nullptr);
        *aCellKindId = _impl.getCellKindIdAtUnchecked(aMemLayout, {aX, aY});
    }

    void _getCellDataAtUnchecked(const ChunkMemoryLayoutInfo& aMemLayout,
                                 hg::PZInteger                aX,
                                 hg::PZInteger                aY,
                                 cell::FloorSprite*           aFloorSprite) const {
        assert(aFloorSprite != nullptr);
        *aFloorSprite = _impl.getFloorSpriteAtUnchecked(aMemLayout, {aX, aY});
    }

    void _getCellDataAtUnchecked(const ChunkMemoryLayoutInfo& aMemLayout,
                                 hg::PZInteger                aX,
                                 hg::PZInteger                aY,
                                 cell::WallSprite*            aWallSprite) const {
        assert(aWallSprite != nullptr);
        *aWallSprite = _impl.getWallSpriteAtUnchecked(aMemLayout, {aX, aY});
    }

    void _getCellDataAtUnchecked(const ChunkMemoryLayoutInfo& aMemLayout,
                                 hg::PZInteger                aX,
                                 hg::PZInteger                aY,
                                 cell::SpatialInfo*           aSpatialInfo) const {
        assert(aSpatialInfo != nullptr);
        *aSpatialInfo = _impl.getSpatialInfoAtUnchecked(aMemLayout, {aX, aY});
    }

    void _getCellDataAtUnchecked(const ChunkMemoryLayoutInfo& aMemLayout,
                                 hg::PZInteger                aX,
                                 hg::PZInteger                aY,
                                 cell::RendererAuxData*       aRendererAuxData) const {
        assert(aRendererAuxData != nullptr);
        *aRendererAuxData = _impl.getRendererAuxDataAtUnchecked(aMemLayout, {aX, aY});
    }

    void _getCellDataAtUnchecked(const ChunkMemoryLayoutInfo& aMemLayout,
                                 hg::PZInteger                aX,
                                 hg::PZInteger                aY,
                                 cell::UserData*              aUserData) const {
        assert(aUserData != nullptr);
        *aUserData = _impl.getUserDataAtUnchecked(aMemLayout, {aX, aY});
    }

    // Setters

    void _setCellDataAtUnchecked(const ChunkMemoryLayoutInfo& aMemLayout,
                                 hg::PZInteger                aX,
                                 hg::PZInteger                aY,
                                 const cell::CellKindId*      aCellKindId) {
        assert(aCellKindId != nullptr);
        _impl.getCellKindIdAtUnchecked(aMemLayout, {aX, aY}) = *aCellKindId;
    }

    void _setCellDataAtUnchecked(const ChunkMemoryLayoutInfo& aMemLayout,
                                 hg::PZInteger                aX,
                                 hg::PZInteger                aY,
                                 const cell::FloorSprite*     aFloorSprite) {
        assert(aFloorSprite != nullptr);
        _impl.getFloorSpriteAtUnchecked(aMemLayout, {aX, aY}) = *aFloorSprite;
    }

    void _setCellDataAtUnchecked(const ChunkMemoryLayoutInfo& aMemLayout,
                                 hg::PZInteger                aX,
                                 hg::PZInteger                aY,
                                 const cell::WallSprite*      aWallSprite) {
        assert(aWallSprite != nullptr);
        _impl.getWallSpriteAtUnchecked(aMemLayout, {aX, aY}) = *aWallSprite;
    }

    void _setCellDataAtUnchecked(const ChunkMemoryLayoutInfo& aMemLayout,
                                 hg::PZInteger                aX,
                                 hg::PZInteger                aY,
                                 const cell::SpatialInfo*     aSpatialInfo) {
        assert(aSpatialInfo != nullptr);
        _impl.getSpatialInfoAtUnchecked(aMemLayout, {aX, aY}) = *aSpatialInfo;
    }

    void _setCellDataAtUnchecked(const ChunkMemoryLayoutInfo& aMemLayout,
                                 hg::PZInteger                aX,
                                 hg::PZInteger                aY,
                                 const cell::RendererAuxData* aRendererAuxData) {
        assert(aRendererAuxData != nullptr);
        _impl.getRendererAuxDataAtUnchecked(aMemLayout, {aX, aY}) = *aRendererAuxData;
    }

    void _setCellDataAtUnchecked(const ChunkMemoryLayoutInfo& aMemLayout,
                                 hg::PZInteger                aX,
                                 hg::PZInteger                aY,
                                 const cell::UserData*        aUserData) {
        assert(aUserData != nullptr);
        _impl.getUserDataAtUnchecked(aMemLayout, {aX, aY}) = *aUserData;
    }
};

static_assert(sizeof(Chunk) == sizeof(void*));

namespace detail {
bool ChunksContainIdenticalCellData(const ChunkMemoryLayoutInfo& aMemLayout,
                                    const Chunk&                 aChunk1,
                                    const Chunk&                 aChunk2);
} // namespace detail

// MARK: Inline impl.

inline bool Chunk::isEmpty() const {
    return _impl.isEmpty();
}

template <class... taPtrs>
inline void Chunk::getCellDataAtUnchecked(const ChunkMemoryLayoutInfo& aMemLayout,
                                          hg::PZInteger                aX,
                                          hg::PZInteger                aY,
                                          taPtrs&&... aPtrs) const {
    static_assert(sizeof...(aPtrs) > 0);

    (_getCellDataAtUnchecked(aMemLayout, aX, aY, aPtrs), ...);
}

template <class... taPtrs>
void Chunk::getCellDataAtUnchecked(const ChunkMemoryLayoutInfo& aMemLayout,
                                   hg::math::Vector2pz          aCell,
                                   taPtrs&&... aPtrs) const {
    static_assert(sizeof...(aPtrs) > 0);

    (_getCellDataAtUnchecked(aMemLayout, aCell.x, aCell.y, aPtrs), ...);
}

inline cell::SpatialInfo Chunk::getSpatialInfoAtUnchecked(const ChunkMemoryLayoutInfo& aMemLayout,
                                                          hg::math::Vector2pz          aCell) const {
    return _impl.getSpatialInfoAtUnchecked(aMemLayout, aCell);
}

template <class... taPtrs>
inline void Chunk::setCellDataAtUnchecked(const ChunkMemoryLayoutInfo& aMemLayout,
                                          hg::PZInteger                aX,
                                          hg::PZInteger                aY,
                                          taPtrs&&... aPtrs) {
    static_assert(sizeof...(aPtrs) > 0);

    (_setCellDataAtUnchecked(aMemLayout, aX, aY, aPtrs), ...);
}

template <class... taPtrs>
inline void Chunk::setCellDataAtUnchecked(const ChunkMemoryLayoutInfo& aMemLayout,
                                          hg::math::Vector2pz          aCell,
                                          taPtrs&&... aPtrs) {
    static_assert(sizeof...(aPtrs) > 0);

    (_setCellDataAtUnchecked(aMemLayout, aCell.x, aCell.y, aPtrs), ...);
}

inline ChunkExtensionInterface* Chunk::getExtension() const {
    return _impl.getExtension();
}

} // namespace gridgoblin
} // namespace jbatnozic
