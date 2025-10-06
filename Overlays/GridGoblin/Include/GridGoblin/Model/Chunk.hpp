// Copyright 2024 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#pragma once

#include <GridGoblin/Model/Cell.hpp>
#include <GridGoblin/Private/Chunk_impl.hpp>

#include <cassert>

namespace jbatnozic {
namespace gridgoblin {

namespace hg = jbatnozic::hobgoblin;

class World;
class ChunkExtensionInterface;

//! TODO(add description)
struct ChunkMemoryLayoutInfo;

class Chunk {
public:
    // clang-format off
    // Default construction (constructs an empty chunk)
    Chunk()                        = default;
    // No copying
    Chunk(const Chunk&)            = delete;
    Chunk& operator=(const Chunk&) = delete;
    // Moving is OK
    Chunk(Chunk&&)                 = default;
    Chunk& operator=(Chunk&&)      = default;
    // clang-format on

    Chunk(const ChunkMemoryLayoutInfo& aMemLayout);

    bool isEmpty() const;

    void makeEmpty();

    ///////////////////////////////////////////////////////////////////////////
    // MARK: CELLS                                                           //
    ///////////////////////////////////////////////////////////////////////////

    //! TODO(add description)
    template <class... taPtrs>
    void getCellDataAtUnchecked(const ChunkMemoryLayoutInfo& aMemLayout,
                                hg::PZInteger                aX,
                                hg::PZInteger                aY,
                                taPtrs&&... aPtrs) const;

    //! TODO(add description)
    template <class... taPtrs>
    void setCellDataAtUnchecked(const ChunkMemoryLayoutInfo& aMemLayout,
                                hg::PZInteger                aX,
                                hg::PZInteger                aY,
                                taPtrs&&... aPtrs);

    //! Set all the cells in the chunk to the given value, unless the chunk is empty. Utility method.
    // void setAll(const CellModel& aCell);

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

    static auto convertMemLayoutInfo(const ChunkMemoryLayoutInfo& aMemLayoutInfo) {
        return reinterpret_cast<const detail::ChunkImpl::MemoryLayoutInfo&>(aMemLayoutInfo);
    }

    // Getters

    void _getCellDataAtUnchecked(const ChunkMemoryLayoutInfo& aMemLayout,
                                 hg::PZInteger                aX,
                                 hg::PZInteger                aY,
                                 cell::CellKindId*            aCellKindId) const {
        assert(aCellKindId != nullptr);
        *aCellKindId = _impl.getCellKindIdAtUnchecked(convertMemLayoutInfo(aMemLayout), {aX, aY});
    }

    void _getCellDataAtUnchecked(const ChunkMemoryLayoutInfo& aMemLayout,
                                 hg::PZInteger                aX,
                                 hg::PZInteger                aY,
                                 cell::FloorSprite*           aFloorSprite) const {
        assert(aFloorSprite != nullptr);
        *aFloorSprite = _impl.getFloorSpriteAtUnchecked(convertMemLayoutInfo(aMemLayout), {aX, aY});
    }

    void _getCellDataAtUnchecked(const ChunkMemoryLayoutInfo& aMemLayout,
                                 hg::PZInteger                aX,
                                 hg::PZInteger                aY,
                                 cell::WallSprite*            aWallSprite) const {
        assert(aWallSprite != nullptr);
        *aWallSprite = _impl.getWallSpriteAtUnchecked(convertMemLayoutInfo(aMemLayout), {aX, aY});
    }

    void _getCellDataAtUnchecked(const ChunkMemoryLayoutInfo& aMemLayout,
                                 hg::PZInteger                aX,
                                 hg::PZInteger                aY,
                                 cell::SpatialInfo*           aSpatialInfo) const {
        assert(aSpatialInfo != nullptr);
        *aSpatialInfo = _impl.getSpatialInfoAtUnchecked(convertMemLayoutInfo(aMemLayout), {aX, aY});
    }

    void _getCellDataAtUnchecked(const ChunkMemoryLayoutInfo& aMemLayout,
                                 hg::PZInteger                aX,
                                 hg::PZInteger                aY,
                                 cell::RendererAuxData*       aRendererAuxData) const {
        assert(aRendererAuxData != nullptr);
        *aRendererAuxData =
            _impl.getRendererAuxDataAtUnchecked(convertMemLayoutInfo(aMemLayout), {aX, aY});
    }

    void _getCellDataAtUnchecked(const ChunkMemoryLayoutInfo& aMemLayout,
                                 hg::PZInteger                aX,
                                 hg::PZInteger                aY,
                                 cell::UserData*              aUserData) const {
        assert(aUserData != nullptr);
        *aUserData = _impl.getUserDataAtUnchecked(convertMemLayoutInfo(aMemLayout), {aX, aY});
    }

    // Setters

    void _setCellDataAtUnchecked(const ChunkMemoryLayoutInfo& aMemLayout,
                                 hg::PZInteger                aX,
                                 hg::PZInteger                aY,
                                 const cell::CellKindId*      aCellKindId) {
        assert(aCellKindId != nullptr);
        _impl.getCellKindIdAtUnchecked(convertMemLayoutInfo(aMemLayout), {aX, aY}) = *aCellKindId;
    }

    void _setCellDataAtUnchecked(const ChunkMemoryLayoutInfo& aMemLayout,
                                 hg::PZInteger                aX,
                                 hg::PZInteger                aY,
                                 const cell::FloorSprite*     aFloorSprite) {
        assert(aFloorSprite != nullptr);
        _impl.getFloorSpriteAtUnchecked(convertMemLayoutInfo(aMemLayout), {aX, aY}) = *aFloorSprite;
    }

    void _setCellDataAtUnchecked(const ChunkMemoryLayoutInfo& aMemLayout,
                                 hg::PZInteger                aX,
                                 hg::PZInteger                aY,
                                 const cell::WallSprite*      aWallSprite) {
        assert(aWallSprite != nullptr);
        _impl.getWallSpriteAtUnchecked(convertMemLayoutInfo(aMemLayout), {aX, aY}) = *aWallSprite;
    }

    void _setCellDataAtUnchecked(const ChunkMemoryLayoutInfo& aMemLayout,
                                 hg::PZInteger                aX,
                                 hg::PZInteger                aY,
                                 const cell::SpatialInfo*     aSpatialInfo) {
        assert(aSpatialInfo != nullptr);
        _impl.getSpatialInfoAtUnchecked(convertMemLayoutInfo(aMemLayout), {aX, aY}) = *aSpatialInfo;
    }

    void _setCellDataAtUnchecked(const ChunkMemoryLayoutInfo& aMemLayout,
                                 hg::PZInteger                aX,
                                 hg::PZInteger                aY,
                                 const cell::RendererAuxData* aRendererAuxData) {
        assert(aRendererAuxData != nullptr);
        _impl.getRendererAuxDataAtUnchecked(convertMemLayoutInfo(aMemLayout), {aX, aY}) =
            *aRendererAuxData;
    }

    void _setCellDataAtUnchecked(const ChunkMemoryLayoutInfo& aMemLayout,
                                 hg::PZInteger                aX,
                                 hg::PZInteger                aY,
                                 const cell::UserData*        aUserData) {
        assert(aUserData != nullptr);
        _impl.getUserDataAtUnchecked(convertMemLayoutInfo(aMemLayout), {aX, aY}) = *aUserData;
    }
};

namespace detail {
bool AreCellsEqual(const Chunk& aChunk1, const Chunk& aChunk2);
} // namespace detail

// MARK: Inline impl.

inline bool Chunk::isEmpty() const {
    return _impl.isEmpty();
}

inline void Chunk::makeEmpty() {
    _impl.makeEmpty();
}

template <class... taPtrs>
inline void Chunk::getCellDataAtUnchecked(const ChunkMemoryLayoutInfo& aMemLayout,
                                          hg::PZInteger                aX,
                                          hg::PZInteger                aY,
                                          taPtrs&&... aPtrs) const {
    (_getCellDataAtUnchecked(aMemLayout, aX, aY, aPtrs), ...);
}

template <class... taPtrs>
inline void Chunk::setCellDataAtUnchecked(const ChunkMemoryLayoutInfo& aMemLayout,
                                          hg::PZInteger                aX,
                                          hg::PZInteger                aY,
                                          taPtrs&&... aPtrs) {
    (_setCellDataAtUnchecked(aMemLayout, aX, aY, aPtrs), ...);
}

inline ChunkExtensionInterface* Chunk::getExtension() const {
    return _impl.getExtension();
}

} // namespace gridgoblin
} // namespace jbatnozic
