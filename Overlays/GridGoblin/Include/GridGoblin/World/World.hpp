// Copyright 2024 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#pragma once

#include <Hobgoblin/Common.hpp>
#include <Hobgoblin/Math.hpp>
#include <Hobgoblin/Utility/Grids.hpp>
#include <Hobgoblin/Utility/Semaphore.hpp>

#include <GridGoblin/Model/Cell.hpp>
#include <GridGoblin/Model/Chunk.hpp>
#include <GridGoblin/Model/Chunk_id.hpp>
#include <GridGoblin/Model/Sprites.hpp>

#include <GridGoblin/World/Active_area.hpp>
#include <GridGoblin/World/Binder.hpp>
#include <GridGoblin/World/World_config.hpp>

#include <GridGoblin/Private/Chunk_holder.hpp>

#include <memory>
#include <optional>
#include <type_traits>
#include <vector>

namespace jbatnozic {
namespace gridgoblin {

namespace hg = jbatnozic::hobgoblin;

class IsometricRenderer;

namespace detail {
class ChunkDiskIoHandlerInterface;
class ChunkSpoolerInterface;
} // namespace detail

/**
 * Grid world!
 * Grid world!
 * Party time!
 * Excellent!
 */
class World : private Binder {
public:
    //! Main constructor.
    World(const WorldConfig& aConfig);

    //! Test constructor.
    //!
    //! \warning This constructor is meant to be used for testing. Do not use it if you are not a
    //!          library maintainer!
    World(const WorldConfig&                                  aConfig,
          hg::NeverNull<detail::ChunkDiskIoHandlerInterface*> aChunkDiskIoHandler);

#ifdef FUTURE
    World(const WorldConfig& aConfig, hg::NeverNull<detail::ChunkSpoolerInterface*> aChunkSpooler);
#endif

    ~World() override;

    static constexpr std::int32_t MIN_BINDER_PRIORITY     = 0;
    static constexpr std::int32_t MAX_BINDER_PRIORITY     = 10'000;
    static constexpr std::int32_t DEFAULT_BINDER_PRIORITY = 100;

    //! Attach a binder to the World.
    //!
    //! \param aBinder binder to attach.
    //! \param aPriority priority of the binder. Must be between
    //!                  `MIN_BINDER_PRIORITY` and `MAX_BINDER_PRIORITY` (inclusive).
    //!
    //! Each attached binder will be notified of the events happening in or to the World, in ascending
    //! order of their priorities (indeterminate when the priorities are equal).
    //!
    //! \note binders are also used to provide chunks extensions to the World. In case that multiple
    //!       binders are attached, the extension from the one with the best (lowest) priority that
    //!       returns a non-null extension will be used.
    //!
    //! \throws hg::TracedLogicError if this binder is already attached.
    //! \throws hg::InvalidArgumentError if priority is out of bounds.
    void attachBinder(hg::NeverNull<Binder*> aBinder, std::int32_t aPriority = DEFAULT_BINDER_PRIORITY);

    //! Detach the binder from the World.
    //!
    //! \param aBinder binder to detach.
    //!
    //! \note Does nothing if the binder is not currently attached to the World.
    void detachBinder(hg::NeverNull<Binder*> aBinder);

    void update();
    void prune();
    void save();

    ///////////////////////////////////////////////////////////////////////////
    // CONVERSIONS                                                           //
    ///////////////////////////////////////////////////////////////////////////

    // Use the following methods to convert from a position in the world to the corresponding
    // cell. For example, if the cell resolution (as defined by the world config) is 32.0f,
    // positions >= 0.0f and < 32.0f belong to cell 0, positions >= 32.0f and < 64.0f belong
    // to cell 1, and so on (the same principle applies to both X and Y axis).

    hg::math::Vector2pz posToCell(double aX, double aY) const;

    hg::math::Vector2pz posToCell(hg::math::Vector2d aPos) const;

    hg::math::Vector2pz posToCellUnchecked(double aX, double aY) const;

    hg::math::Vector2pz posToCellUnchecked(hg::math::Vector2d aPos) const;

    // Use the following methods to convert from the coordinates of a single cell to the
    // corresponding chunk. For example, if the chunk width (as defined by the world config) is
    // 8, then cells with coordinates [0, y]-[7, y] belong to chunk [0, Y]; cells with
    // coordinates [8, y]-[15, y] belong to chunk [1, Y]; and so on. If the chunk height
    // is also 8, then cells with coordinates [x, 0]-[x, 7] belong to chunk [X, 0]; and so on.

    ChunkId cellToChunkId(hg::PZInteger aX, hg::PZInteger aY) const;

    ChunkId cellToChunkId(hg::math::Vector2pz aCell) const;

    ChunkId cellToChunkIdUnchecked(hg::PZInteger aX, hg::PZInteger aY) const;

    ChunkId cellToChunkIdUnchecked(hg::math::Vector2pz aCell) const;

    ///////////////////////////////////////////////////////////////////////////
    // LOCKING                                                               //
    ///////////////////////////////////////////////////////////////////////////

    class EditPermission {
    public:
        ~EditPermission();

    private:
        EditPermission() = default;
        friend class World;
    };

    std::unique_ptr<EditPermission> getPermissionToEdit();

    ///////////////////////////////////////////////////////////////////////////
    // MARK: CELL GETTERS                                                    //
    ///////////////////////////////////////////////////////////////////////////

    double getCellResolution() const;

    double getWallHeight() const;

    hg::PZInteger getCellCountX() const;

    hg::PZInteger getCellCountY() const;

    // Without locking (no on-demand loading)

    template <class... taPtrs>
    [[nodiscard]] bool getCellDataAt(hg::PZInteger aX, hg::PZInteger aY, taPtrs&&... aPtrs) const;

    template <class... taPtrs>
    [[nodiscard]] bool getCellDataAt(hg::math::Vector2pz aCell, taPtrs&&... aPtrs) const;

    template <class... taPtrs>
    [[nodiscard]] bool getCellDataAtUnchecked(hg::PZInteger aX,
                                              hg::PZInteger aY,
                                              taPtrs&&... aPtrs) const;

    template <class... taPtrs>
    [[nodiscard]] bool getCellDataAtUnchecked(hg::math::Vector2pz aCell, taPtrs&&... aPtrs) const;

    // With locking (on-demand loading enabled)

    template <class... taPtrs>
    void getCellDataAt(const EditPermission& aPerm,
                       hg::PZInteger         aX,
                       hg::PZInteger         aY,
                       taPtrs&&... aPtrs) const;

    template <class... taPtrs>
    void getCellDataAt(const EditPermission& aPerm, hg::math::Vector2pz aCell, taPtrs&&... aPtrs) const;

    template <class... taPtrs>
    void getCellDataAtUnchecked(const EditPermission& aPerm,
                                hg::PZInteger         aX,
                                hg::PZInteger         aY,
                                taPtrs&&... aPtrs) const;

    template <class... taPtrs>
    void getCellDataAtUnchecked(const EditPermission& aPerm,
                                hg::math::Vector2pz   aCell,
                                taPtrs&&... aPtrs) const;

    ///////////////////////////////////////////////////////////////////////////
    // MARK: CELL UPDATERS                                                   //
    ///////////////////////////////////////////////////////////////////////////

    //! Generator mode is intended for very long world edits which are expected to touch more chunks
    //! that what is allowed to be loaded at a time (`WorldConfig::maxLoadedNonessentialChunks`), such
    //! as procedural terrain generation - hence the name.
    //!
    //! While generator mode is on, the World instance is allowed to unload chunks *during* editing in
    //! order to respect this limit, AND cells won't be refreshed after editing (meaning that their
    //! openness values will likely be wrong; rendering will also not work properly).
    //!
    //! Once generator mode is toggled off, all cells in all remaining currently loaded chunks will
    //! be refreshed.
    //!
    //! \note nothing can make the World instance unload chunks which are in one or more Active Areas,
    //!       and this also holds even while generator mode is toggled on.
    void toggleGeneratorMode(const EditPermission&, bool aGeneratorMode);

    class Editor {
    public:
        template <class... taPtrs>
        void setCellDataAt(hg::PZInteger aX, hg::PZInteger aY, taPtrs&&... aPtrs);

        template <class... taPtrs>
        void setCellDataAt(hg::math::Vector2pz aCell, taPtrs&&... aPtrs);

        template <class... taPtrs>
        void setCellDataAtUnchecked(hg::PZInteger aX, hg::PZInteger aY, taPtrs&&... aPtrs);

        template <class... taPtrs>
        void setCellDataAtUnchecked(hg::math::Vector2pz aCell, taPtrs&&... aPtrs);

    private:
        friend class World;
        Editor(World& aWorld)
            : _world{aWorld} {}
        World& _world;
    };

    // TODO: how to solve cache overflow in very large edits?
    template <class taCallable>
    void edit(const EditPermission&, taCallable&& aCallable) {
        _startEdit();
        Editor editor{*this};
        try {
            aCallable(editor);
        } catch (...) {
            _endEdit();
            throw;
        }
        _endEdit();
    }

    ///////////////////////////////////////////////////////////////////////////
    // MARK: CHUNKS                                                          //
    ///////////////////////////////////////////////////////////////////////////

    BuildingBlockMask getBuildingBlocks() const;

    hg::PZInteger getChunkCountX() const;

    hg::PZInteger getChunkCountY() const;

    hg::PZInteger getCellsPerChunkX() const;

    hg::PZInteger getCellsPerChunkY() const;

    const ChunkMemoryLayoutInfo& getChunkMemoryLayoutInfo() const;

    // Without locking (no on-demand loading)

    const Chunk* getChunkAt(hg::PZInteger aX, hg::PZInteger aY) const;

    const Chunk* getChunkAtUnchecked(hg::PZInteger aX, hg::PZInteger aY) const;

    const Chunk* getChunkAt(hg::math::Vector2pz aCell) const;

    const Chunk* getChunkAtUnchecked(hg::math::Vector2pz aCell) const;

    const Chunk* getChunkAtId(ChunkId aChunkId) const;

    const Chunk* getChunkAtIdUnchecked(ChunkId aChunkId) const;

    // With locking (on-demand loading enabled)

    const Chunk& getChunkAt(const EditPermission& aPerm, hg::PZInteger aX, hg::PZInteger aY) const;

    const Chunk& getChunkAtUnchecked(const EditPermission& aPerm,
                                     hg::PZInteger         aX,
                                     hg::PZInteger         aY) const;

    const Chunk& getChunkAt(const EditPermission& aPerm, hg::math::Vector2pz aCell) const;

    const Chunk& getChunkAtUnchecked(const EditPermission& aPerm, hg::math::Vector2pz aCell) const;

    const Chunk& getChunkAtId(const EditPermission& aPerm, ChunkId aChunkId) const;

    const Chunk& getChunkAtIdUnchecked(const EditPermission& aPerm, ChunkId aChunkId) const;

    //! Iterator-like object used to traverse through all the chunks that are currently loaded
    //! and available through the world instance. The order of traversal throught he chunks is
    //! indeterminate and should be considered random.
    class AvailableChunkIterator {
    public:
        using value_type = ChunkId;

        void advance() {
            _impl.advance();
        }

        AvailableChunkIterator& operator++() {
            advance();
            return *this;
        }

        value_type operator*() const {
            return _impl.dereference();
        }

        bool operator==(const AvailableChunkIterator& aOther) const {
            return _impl.equals(aOther._impl);
        }

    private:
        friend class World;

        AvailableChunkIterator(detail::ChunkHolder::AvailableChunkIterator aImpl)
            : _impl{aImpl} {}

        detail::ChunkHolder::AvailableChunkIterator _impl;
    };

    //! Begin interating through available chunks.
    //!
    //! \warning the returned iterator, as well as all of its copies and modifications remain valid only
    //!          so long as the world is not edited (editing means calling any function that requires
    //!          an `EditPermission` as a parameter). Using an invalid iterator in any way, other than
    //!          to destroy it, can lead to unexpected results, which can include crashes.
    AvailableChunkIterator availableChunksBegin() const {
        return {_chunkStorage.availableChunksBegin()};
    }

    //! Obtain an end iterator to compare the others to while iterating.
    //!
    //! \warning the returned iterator, as well as all of its copies and modifications remain valid only
    //!          so long as the world is not edited (editing means calling any function that requires
    //!          an `EditPermission` as a parameter). Using an invalid iterator in any way, other than
    //!          to destroy it, can lead to unexpected results, which can include crashes.
    AvailableChunkIterator availableChunksEnd() const {
        return {_chunkStorage.availableChunksEnd()};
    }

    ///////////////////////////////////////////////////////////////////////////
    // MARK: ACTIVE AREAS                                                    //
    ///////////////////////////////////////////////////////////////////////////

    ActiveArea createActiveArea();

private:
    // ===== Listeners =====

    std::vector<std::pair<hg::NeverNull<Binder*>, std::int32_t>> _binders;

    void _attachBinder(hg::NeverNull<Binder*> aBinder, std::int32_t aPriority);

    // ===== Config =====

    struct WorldConfigExt : WorldConfig {
        hg::PZInteger cellCountX;
        hg::PZInteger cellCountY;

        WorldConfigExt(const WorldConfig& aConfig)
            : WorldConfig{aConfig}
            , cellCountX{chunkCountX * cellsPerChunkX}
            , cellCountY{chunkCountY * cellsPerChunkY} {}
    };

    WorldConfigExt _config;

    // ===== Subcomponents =====

    detail::ChunkHolder _chunkStorage;

    std::unique_ptr<detail::ChunkDiskIoHandlerInterface> _internalChunkDiskIoHandler;
    detail::ChunkDiskIoHandlerInterface*                 _chunkDiskIoHandler;

    std::unique_ptr<detail::ChunkSpoolerInterface> _internalChunkSpooler;
    hg::NeverNull<detail::ChunkSpoolerInterface*>  _chunkSpooler;

    void _connectSubcomponents();
    void _disconnectSubcomponents();

    // ===== Callbacks =====

    void _refreshCellsInAndAroundChunk(ChunkId aChunkId);

    void onChunkReady(ChunkId aChunkId) override;
    void onChunkLoaded(ChunkId aChunkId, const Chunk& aChunk) override;
    void onChunkCreated(ChunkId aChunkId, const Chunk& aChunk) override;
    void onChunkUnloaded(ChunkId aChunkId) override;
    std::unique_ptr<ChunkExtensionInterface> createChunkExtension() override;

    // ===== Editing cells =====

    std::vector<Binder::CellEditInfo> _cellEditInfos;

    int _editMinX = -1;
    int _editMinY = -1;
    int _editMaxX = -1;
    int _editMaxY = -1;

    bool _isInEditMode      = false;
    bool _isInGeneratorMode = false;

    void _startEdit();
    void _endEdit();

    struct RingAssessment {
        bool hasOccupiedCells = false;
        bool extend           = false;
    };

    template <bool taAllowedToLoadAdjacent>
    std::optional<cell::SpatialInfo> _getSpatialInfoOfCellAtUnchecked(hg::PZInteger aX,
                                                                      hg::PZInteger aY) const;

    template <bool taAllowedToLoadAdjacent>
    RingAssessment _assessRing(hg::PZInteger aX, hg::PZInteger aY, hg::PZInteger aRing);

    // TODO: since we determine openness at chunk load, this will lead into endless chunk load loop
    template <bool taAllowedToLoadAdjacent>
    hg::PZInteger _calcOpennessAt(hg::PZInteger aX, hg::PZInteger aY);

    void _refreshCellAtUnchecked(hg::PZInteger aX, hg::PZInteger aY);

    //! \param aChunk chunk where the `aX` and `aY` coordinates lie
    //! \param aX absolute X coordinate of the cell (NOT relative to chunk top-left)
    //! \param aY absolute Y coordinate of the cell (NOT relative to chunk top-left)
    //! \param aSpatialInfo pointer to SpatialInfo object to copy over (UB if null)
    void _setCellDataAtUnchecked(Chunk&                   aChunk,
                                 hg::PZInteger            aX,
                                 hg::PZInteger            aY,
                                 const cell::SpatialInfo* aSpatialInfo);

    //! \param aChunk chunk where the `aX` and `aY` coordinates lie
    //! \param aX absolute X coordinate of the cell (NOT relative to chunk top-left)
    //! \param aY absolute Y coordinate of the cell (NOT relative to chunk top-left)
    template <class T, typename std::enable_if_t<!std::is_same_v<T, cell::SpatialInfo>, bool> = true>
    void _setCellDataAtUnchecked(Chunk& aChunk, hg::PZInteger aX, hg::PZInteger aY, const T* aPtr);
};

///////////////////////////////////////////////////////////////////////////
// CELL GETTERS                                                          //
///////////////////////////////////////////////////////////////////////////

template <class... taPtrs>
[[nodiscard]] bool World::getCellDataAt(hg::PZInteger aX, hg::PZInteger aY, taPtrs&&... aPtrs) const {
    static_assert(sizeof...(aPtrs) > 0);

    // TODO: validate args
    return getCellDataAtUnchecked(aX, aY, std::forward<taPtrs>(aPtrs)...);
}

template <class... taPtrs>
[[nodiscard]] bool World::getCellDataAt(hg::math::Vector2pz aCell, taPtrs&&... aPtrs) const {
    static_assert(sizeof...(aPtrs) > 0);

    // TODO: validate args
    return getCellDataAtUnchecked(aCell.x, aCell.y, std::forward<taPtrs>(aPtrs)...);
}

template <class... taPtrs>
[[nodiscard]] bool World::getCellDataAtUnchecked(hg::PZInteger aX,
                                                 hg::PZInteger aY,
                                                 taPtrs&&... aPtrs) const {
    static_assert(sizeof...(aPtrs) > 0);

    const auto chunkX = aX / _config.cellsPerChunkX;
    const auto chunkY = aY / _config.cellsPerChunkY;

    if (auto* chunk = _chunkStorage.getChunkAtIdUnchecked({chunkX, chunkY}); chunk) {
        chunk->getCellDataAtUnchecked(getChunkMemoryLayoutInfo(),
                                      aX % _config.cellsPerChunkX,
                                      aY % _config.cellsPerChunkY,
                                      std::forward<taPtrs>(aPtrs)...);
        return true;
    }

    return false;
}

template <class... taPtrs>
[[nodiscard]] bool World::getCellDataAtUnchecked(hg::math::Vector2pz aCell, taPtrs&&... aPtrs) const {
    static_assert(sizeof...(aPtrs) > 0);

    return getCellDataAtUnchecked(aCell.x, aCell.y, std::forward<taPtrs>(aPtrs)...);
}

template <class... taPtrs>
void World::getCellDataAt(const EditPermission& aPerm,
                          hg::PZInteger         aX,
                          hg::PZInteger         aY,
                          taPtrs&&... aPtrs) const {
    static_assert(sizeof...(aPtrs) > 0);

    // TODO: validate args
    getCellDataAtUnchecked(aPerm, aX, aY, std::forward<taPtrs>(aPtrs)...);
}

template <class... taPtrs>
void World::getCellDataAt(const EditPermission& aPerm,
                          hg::math::Vector2pz   aCell,
                          taPtrs&&... aPtrs) const {
    static_assert(sizeof...(aPtrs) > 0);

    // TODO: validate args
    getCellDataAtUnchecked(aPerm, aCell.x, aCell.y, std::forward<taPtrs>(aPtrs)...);
}

template <class... taPtrs>
void World::getCellDataAtUnchecked(const EditPermission& aPerm,
                                   hg::PZInteger         aX,
                                   hg::PZInteger         aY,
                                   taPtrs&&... aPtrs) const {
    static_assert(sizeof...(aPtrs) > 0);

    const auto chunkX = aX / _config.cellsPerChunkX;
    const auto chunkY = aY / _config.cellsPerChunkY;

    const auto& chunk = _chunkStorage.getChunkAtIdUnchecked({chunkX, chunkY}, detail::LOAD_IF_MISSING);
    chunk.getCellDataAtUnchecked(getChunkMemoryLayoutInfo(),
                                 aX % _config.cellsPerChunkX,
                                 aY % _config.cellsPerChunkY,
                                 std::forward<taPtrs>(aPtrs)...);
}

template <class... taPtrs>
void World::getCellDataAtUnchecked(const EditPermission& aPerm,
                                   hg::math::Vector2pz   aCell,
                                   taPtrs&&... aPtrs) const {
    static_assert(sizeof...(aPtrs) > 0);

    getCellDataAtUnchecked(aPerm, aCell.x, aCell.y, std::forward<taPtrs>(aPtrs)...);
}

///////////////////////////////////////////////////////////////////////////
// CELL UPDATES                                                          //
///////////////////////////////////////////////////////////////////////////

template <class... taPtrs>
void World::Editor::setCellDataAt(hg::PZInteger aX, hg::PZInteger aY, taPtrs&&... aPtrs) {
    static_assert(sizeof...(aPtrs) > 0);

    // TODO: validate args
    setCellDataAtUnchecked(aX, aY, std::forward<taPtrs>(aPtrs)...);
}

template <class... taPtrs>
void World::Editor::setCellDataAt(hg::math::Vector2pz aCell, taPtrs&&... aPtrs) {
    static_assert(sizeof...(aPtrs) > 0);

    // TODO: validate args
    setCellDataAtUnchecked(aCell.x, aCell.y, std::forward<taPtrs>(aPtrs)...);
}

template <class... taPtrs>
void World::Editor::setCellDataAtUnchecked(hg::PZInteger aX, hg::PZInteger aY, taPtrs&&... aPtrs) {
    static_assert(sizeof...(aPtrs) > 0);

    const auto chunkX = aX / _world._config.cellsPerChunkX;
    const auto chunkY = aY / _world._config.cellsPerChunkY;

    auto& chunk = _world._chunkStorage.getChunkAtIdUnchecked({chunkX, chunkY}, detail::LOAD_IF_MISSING);
    (_world._setCellDataAtUnchecked(chunk, aX, aY, aPtrs), ...);
}

template <class... taPtrs>
void World::Editor::setCellDataAtUnchecked(hg::math::Vector2pz aCell, taPtrs&&... aPtrs) {
    static_assert(sizeof...(aPtrs) > 0);

    setCellDataAtUnchecked(aCell.x, aCell.y, std::forward<taPtrs>(aPtrs)...);
}

template <class T, typename std::enable_if_t<!std::is_same_v<T, cell::SpatialInfo>, bool>>
void World::_setCellDataAtUnchecked(Chunk& aChunk, hg::PZInteger aX, hg::PZInteger aY, const T* aPtr) {
    const auto cellRelativeX = aX % _config.cellsPerChunkX;
    const auto cellRelativeY = aY % _config.cellsPerChunkY;
    aChunk.setCellDataAtUnchecked(getChunkMemoryLayoutInfo(), cellRelativeX, cellRelativeY, aPtr);
}

inline BuildingBlockMask World::getBuildingBlocks() const {
    return _config.buildingBlocks;
}

inline hg::PZInteger World::getChunkCountX() const {
    return _chunkStorage.getChunkCountX();
}

inline hg::PZInteger World::getChunkCountY() const {
    return _chunkStorage.getChunkCountY();
}

inline hg::PZInteger World::getCellsPerChunkX() const {
    return _config.cellsPerChunkX;
}

inline hg::PZInteger World::getCellsPerChunkY() const {
    return _config.cellsPerChunkY;
}

inline const ChunkMemoryLayoutInfo& World::getChunkMemoryLayoutInfo() const {
    return reinterpret_cast<const ChunkMemoryLayoutInfo&>(_chunkStorage.GetChunkMemoryLayoutInfo());
}

} // namespace gridgoblin
} // namespace jbatnozic
