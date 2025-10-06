// Copyright 2024 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#pragma once

#include <GridGoblin/Model/Building_block.hpp>
#include <GridGoblin/Model/Cell.hpp>

#include <Hobgoblin/Common.hpp>
#include <Hobgoblin/HGExcept.hpp>
#include <Hobgoblin/Math/Vector2.hpp>

#include <cstddef>
#include <memory>

namespace jbatnozic {
namespace gridgoblin {

class ChunkExtensionInterface;

namespace detail {

namespace hg = ::jbatnozic::hobgoblin;

class ChunkImpl {
public:
    struct MemoryLayoutInfo {
        hg::PZInteger chunkWidth;  //!< Number of columns in the chunk
        hg::PZInteger chunkHeight; //!< Number of rows in the chunk

        struct Offsets {
            std::size_t cellKindId      = static_cast<std::size_t>(-1);
            std::size_t floorSprite     = static_cast<std::size_t>(-1);
            std::size_t wallSprite      = static_cast<std::size_t>(-1);
            std::size_t spatialInfo     = static_cast<std::size_t>(-1);
            std::size_t rendererAuxData = static_cast<std::size_t>(-1);
            std::size_t userData        = static_cast<std::size_t>(-1);
        };

        Offsets     offset;
        std::size_t totalSize;
    };

    static MemoryLayoutInfo calcMemoryLayoutInfo(hg::PZInteger     aChunkWidth,
                                                 hg::PZInteger     aChunkHeight,
                                                 BuildingBlockMask aBuildingBlocks);

    ChunkImpl() = default;

    ChunkImpl(const ChunkImpl& aOther)            = delete;
    ChunkImpl& operator=(const ChunkImpl& aOther) = delete;

    ChunkImpl(ChunkImpl&& aOther);
    ChunkImpl& operator=(ChunkImpl&& aOther);

    ~ChunkImpl();

    void init(const MemoryLayoutInfo& aMemLayout);

    bool isEmpty() const {
        return (_mem == nullptr);
    }

    void makeEmpty();

    ///////////////////////////////////////////////////////////////////////////
    // MARK: CELL DATA                                                       //
    ///////////////////////////////////////////////////////////////////////////

    // CellKindId

    cell::CellKindId& getCellKindIdAtUnchecked(const MemoryLayoutInfo& aMemLayout,
                                               hg::math::Vector2pz     aCellPos);

    const cell::CellKindId& getCellKindIdAtUnchecked(const MemoryLayoutInfo& aMemLayout,
                                                     hg::math::Vector2pz     aCellPos) const;

    // FloorSprite

    cell::FloorSprite& getFloorSpriteAtUnchecked(const MemoryLayoutInfo& aMemLayout,
                                                 hg::math::Vector2pz     aCellPos);

    const cell::FloorSprite& getFloorSpriteAtUnchecked(const MemoryLayoutInfo& aMemLayout,
                                                       hg::math::Vector2pz     aCellPos) const;

    // WallSprite

    cell::WallSprite& getWallSpriteAtUnchecked(const MemoryLayoutInfo& aMemLayout,
                                               hg::math::Vector2pz     aCellPos);

    const cell::WallSprite& getWallSpriteAtUnchecked(const MemoryLayoutInfo& aMemLayout,
                                                     hg::math::Vector2pz     aCellPos) const;

    // SpatialInfo

    cell::SpatialInfo& getSpatialInfoAtUnchecked(const MemoryLayoutInfo& aMemLayout,
                                                 hg::math::Vector2pz     aCellPos);

    const cell::SpatialInfo& getSpatialInfoAtUnchecked(const MemoryLayoutInfo& aMemLayout,
                                                       hg::math::Vector2pz     aCellPos) const;

    // RendererAuxData

    cell::RendererAuxData& getRendererAuxDataAtUnchecked(const MemoryLayoutInfo& aMemLayout,
                                                         hg::math::Vector2pz     aCellPos);

    const cell::RendererAuxData& getRendererAuxDataAtUnchecked(const MemoryLayoutInfo& aMemLayout,
                                                               hg::math::Vector2pz     aCellPos) const;

    // UserData

    cell::UserData& getUserDataAtUnchecked(const MemoryLayoutInfo& aMemLayout,
                                           hg::math::Vector2pz     aCellPos);

    const cell::UserData& getUserDataAtUnchecked(const MemoryLayoutInfo& aMemLayout,
                                                 hg::math::Vector2pz     aCellPos) const;

    ///////////////////////////////////////////////////////////////////////////
    // MARK: EXTENSION                                                       //
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
    std::byte* _mem = nullptr;

    void _storeChunkExtensionPointer(ChunkExtensionInterface* aChunkExtensionPointer);
    ChunkExtensionInterface* _loadChunkExtensionPointer() const;
};

// MARK: Inline getter definitions

// CellKindId

inline cell::CellKindId& ChunkImpl::getCellKindIdAtUnchecked(const MemoryLayoutInfo& aMemLayout,
                                                             hg::math::Vector2pz     aCellPos) {
    auto* array = reinterpret_cast<cell::CellKindId*>(_mem + aMemLayout.offset.cellKindId);
    return array[(aCellPos.y * aMemLayout.chunkWidth) + aCellPos.x];
}

inline const cell::CellKindId& ChunkImpl::getCellKindIdAtUnchecked(const MemoryLayoutInfo& aMemLayout,
                                                                   hg::math::Vector2pz aCellPos) const {
    return const_cast<ChunkImpl*>(this)->getCellKindIdAtUnchecked(aMemLayout, aCellPos);
}

// FloorSprite

inline cell::FloorSprite& ChunkImpl::getFloorSpriteAtUnchecked(const MemoryLayoutInfo& aMemLayout,
                                                               hg::math::Vector2pz     aCellPos) {
    auto* array = reinterpret_cast<cell::FloorSprite*>(_mem + aMemLayout.offset.floorSprite);
    return array[(aCellPos.y * aMemLayout.chunkWidth) + aCellPos.x];
}

inline const cell::FloorSprite& ChunkImpl::getFloorSpriteAtUnchecked(
    const MemoryLayoutInfo& aMemLayout,
    hg::math::Vector2pz     aCellPos) const //
{
    return const_cast<ChunkImpl*>(this)->getFloorSpriteAtUnchecked(aMemLayout, aCellPos);
}

// WallSprite

inline cell::WallSprite& ChunkImpl::getWallSpriteAtUnchecked(const MemoryLayoutInfo& aMemLayout,
                                                             hg::math::Vector2pz     aCellPos) {
    auto* array = reinterpret_cast<cell::WallSprite*>(_mem + aMemLayout.offset.wallSprite);
    return array[(aCellPos.y * aMemLayout.chunkWidth) + aCellPos.x];
}

inline const cell::WallSprite& ChunkImpl::getWallSpriteAtUnchecked(const MemoryLayoutInfo& aMemLayout,
                                                                   hg::math::Vector2pz aCellPos) const {
    return const_cast<ChunkImpl*>(this)->getWallSpriteAtUnchecked(aMemLayout, aCellPos);
}

// SpatialInfo

inline cell::SpatialInfo& ChunkImpl::getSpatialInfoAtUnchecked(const MemoryLayoutInfo& aMemLayout,
                                                               hg::math::Vector2pz     aCellPos) {
    auto* array = reinterpret_cast<cell::SpatialInfo*>(_mem + aMemLayout.offset.spatialInfo);
    return array[(aCellPos.y * aMemLayout.chunkWidth) + aCellPos.x];
}

inline const cell::SpatialInfo& ChunkImpl::getSpatialInfoAtUnchecked(
    const MemoryLayoutInfo& aMemLayout,
    hg::math::Vector2pz     aCellPos) const //
{
    return const_cast<ChunkImpl*>(this)->getSpatialInfoAtUnchecked(aMemLayout, aCellPos);
}

// RendererAuxData

inline cell::RendererAuxData& ChunkImpl::getRendererAuxDataAtUnchecked(
    const MemoryLayoutInfo& aMemLayout,
    hg::math::Vector2pz     aCellPos) //
{
    auto* array = reinterpret_cast<cell::RendererAuxData*>(_mem + aMemLayout.offset.rendererAuxData);
    return array[(aCellPos.y * aMemLayout.chunkWidth) + aCellPos.x];
}

inline const cell::RendererAuxData& ChunkImpl::getRendererAuxDataAtUnchecked(
    const MemoryLayoutInfo& aMemLayout,
    hg::math::Vector2pz     aCellPos) const //
{
    return const_cast<ChunkImpl*>(this)->getRendererAuxDataAtUnchecked(aMemLayout, aCellPos);
}

// UserData

inline cell::UserData& ChunkImpl::getUserDataAtUnchecked(const MemoryLayoutInfo& aMemLayout,
                                                         hg::math::Vector2pz     aCellPos) {
    auto* array = reinterpret_cast<cell::UserData*>(_mem + aMemLayout.offset.userData);
    return array[(aCellPos.y * aMemLayout.chunkWidth) + aCellPos.x];
}

inline const cell::UserData& ChunkImpl::getUserDataAtUnchecked(const MemoryLayoutInfo& aMemLayout,
                                                               hg::math::Vector2pz     aCellPos) const {
    return const_cast<ChunkImpl*>(this)->getUserDataAtUnchecked(aMemLayout, aCellPos);
}

} // namespace detail

} // namespace gridgoblin
} // namespace jbatnozic
