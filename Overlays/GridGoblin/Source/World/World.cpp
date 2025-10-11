// Copyright 2024 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#include <GridGoblin/Private/Chunk_disk_io_handler_default.hpp>
#include <GridGoblin/Private/Chunk_disk_io_handler_interface.hpp>
#include <GridGoblin/Private/Chunk_spooler_default.hpp>
#include <GridGoblin/Private/Chunk_spooler_interface.hpp>
#include <GridGoblin/World/World.hpp>

#include <Hobgoblin/HGExcept.hpp>

#include <algorithm>
#include <cmath>

namespace jbatnozic {
namespace gridgoblin {

namespace {
std::unique_ptr<detail::ChunkDiskIoHandlerInterface> CreateDiskIoHandler(const WorldConfig& aConfig) {
    return std::make_unique<detail::DefaultChunkDiskIoHandler>(aConfig);
}

std::unique_ptr<detail::ChunkSpoolerInterface> CreateChunkSpooler(
    const WorldConfig&           aConfig,
    const ChunkMemoryLayoutInfo& aChunkMemLayout) //
{
    return std::make_unique<detail::DefaultChunkSpooler>(aConfig.buildingBlocks, aChunkMemLayout);
}
} // namespace

World::World(const WorldConfig& aConfig)
    : _config{WorldConfig::validate(aConfig)}
    , _chunkHolder{aConfig}
    , _internalChunkDiskIoHandler{CreateDiskIoHandler(aConfig)}
    , _chunkDiskIoHandler{_internalChunkDiskIoHandler.get()}
    , _internalChunkSpooler{CreateChunkSpooler(aConfig, getChunkMemoryLayoutInfo())}
    , _chunkSpooler{_internalChunkSpooler.get()} //
{
    _connectSubcomponents();
}

World::World(const WorldConfig&                                  aConfig,
             hg::NeverNull<detail::ChunkDiskIoHandlerInterface*> aChunkDiskIoHandler)
    : _config{WorldConfig::validate(aConfig)}
    , _chunkHolder{aConfig}
    , _internalChunkDiskIoHandler{nullptr}
    , _chunkDiskIoHandler{aChunkDiskIoHandler}
    , _internalChunkSpooler{CreateChunkSpooler(aConfig, getChunkMemoryLayoutInfo())}
    , _chunkSpooler{_internalChunkSpooler.get()} //
{
    _connectSubcomponents();
}

#ifdef FUTURE
World::World(const WorldConfig& aConfig, hg::NeverNull<detail::ChunkSpoolerInterface*> aChunkSpooler)
    : _config{WorldConfig::validate(aConfig)}
    , _chunkDiskIoHandler{nullptr}
    , _chunkSpooler{nullptr}
    , _chunkHolder{*aChunkSpooler, aConfig} {}
#endif

World::~World() {
    _disconnectSubcomponents();
}

void World::attachBinder(hg::NeverNull<Binder*> aBinder, std::int32_t aPriority) {
    HG_VALIDATE_ARGUMENT(aPriority >= MIN_BINDER_PRIORITY && aPriority <= MAX_BINDER_PRIORITY);
    _attachBinder(aBinder, aPriority);
}

void World::detachBinder(hg::NeverNull<Binder*> aBinder) {
    const auto iter = std::find_if(_binders.begin(), _binders.end(), [aBinder](auto& aPair) {
        return (aPair.first == aBinder);
    });
    if (iter != _binders.end()) {
        _binders.erase(iter);
    }
}

///////////////////////////////////////////////////////////////////////////
// MARK: TEMPLATES                                                       //
///////////////////////////////////////////////////////////////////////////

template <bool taAllowedToLoadAdjacent>
std::optional<cell::SpatialInfo> World::_getSpatialInfoOfCellAtUnchecked(hg::PZInteger aX,
                                                                         hg::PZInteger aY) const {
    const Chunk* chunk;

    if constexpr (taAllowedToLoadAdjacent) {
        chunk = &_chunkHolder.getChunkAtIdUnchecked(
            {aX / _config.cellsPerChunkX, aY / _config.cellsPerChunkY},
            detail::LOAD_IF_MISSING);
    } else {
        chunk = _chunkHolder.getChunkAtIdUnchecked(
            {aX / _config.cellsPerChunkX, aY / _config.cellsPerChunkY});
    }

    if (!chunk) {
        return std::nullopt;
    }

    cell::SpatialInfo spatialInfo;
    chunk->getCellDataAtUnchecked(getChunkMemoryLayoutInfo(),
                                  aX % _config.cellsPerChunkX,
                                  aY % _config.cellsPerChunkY,
                                  &spatialInfo);
    return spatialInfo;
}

template <bool taAllowedToLoadAdjacent>
World::RingAssessment World::_assessRing(hg::PZInteger aX, hg::PZInteger aY, hg::PZInteger aRing) {
    RingAssessment result;

    bool hasTopSide    = false;
    bool hasLeftSide   = false;
    bool hasRightSide  = false;
    bool hasBottomSide = false;

    const auto isCellSolid = [this](hg::PZInteger aX, hg::PZInteger aY) -> bool {
        if (aX < 0 || aX >= _config.cellCountX || aY < 0 || aY >= _config.cellCountY) {
            return true;
        }

        const auto spatialInfo = _getSpatialInfoOfCellAtUnchecked<taAllowedToLoadAdjacent>(aX, aY);
        return !spatialInfo || spatialInfo->hasNonEmptyWallShape();
    };

    if (aRing == 0) {
        const bool solid = isCellSolid(aX, aY);
        if (solid) {
            result.hasOccupiedCells = true;
        }
        return result;
    }

    // Check top row (except corners)
    {
        const int y = aY - aRing;

        for (int x = aX - aRing + 1; x <= aX + aRing - 1; x += 1) {
            const bool solid = isCellSolid(x, y);
            if (solid) {
                result.hasOccupiedCells = true;
                hasTopSide              = true;
            }
        }
    }
    // Check bottom row (except corners)
    {
        const int y = aY + aRing;

        for (int x = aX - aRing + 1; x <= aX + aRing - 1; x += 1) {
            const bool solid = isCellSolid(x, y);
            if (solid) {
                result.hasOccupiedCells = true;
                hasBottomSide           = true;

                // Possible early exit
                if (hasTopSide) {
                    return result;
                }
            }
        }
    }
    // Check middle rows (only extremes)
    {
        for (int y = aY - aRing + 1; y <= aY + aRing - 1; y += 1) {
            {
                const int  x     = aX - aRing;
                const bool solid = isCellSolid(x, y);
                if (solid) {
                    result.hasOccupiedCells = true;
                    hasLeftSide             = true;

                    // Possible early exit
                    if (hasRightSide) {
                        return result;
                    }
                }
            }
            {
                const int  x     = aX + aRing;
                const bool solid = isCellSolid(x, y);
                if (solid) {
                    result.hasOccupiedCells = true;
                    hasRightSide            = true;

                    // Possible early exit
                    if (hasLeftSide) {
                        return result;
                    }
                }
            }
        }
    }
    // Check corners
    {
        // TopLeft, TopRight, BottomLeft, BottomRight
        const bool tl = isCellSolid(aX - aRing, aY - aRing);
        const bool tr = isCellSolid(aX + aRing, aY - aRing);
        const bool bl = isCellSolid(aX - aRing, aY + aRing);
        const bool br = isCellSolid(aX + aRing, aY + aRing);

        const hg::PZInteger delta = hg::ToPz(tl) + hg::ToPz(tr) + hg::ToPz(bl) + hg::ToPz(br);
        result.hasOccupiedCells |= (delta > 0);

        if (delta == 4) {
            return result;
        }

        // clang-format off
        if ((!tl && !hasTopSide    && !hasLeftSide)  ||
            (!tr && !hasTopSide    && !hasRightSide) ||
            (!bl && !hasBottomSide && !hasLeftSide)  ||
            (!br && !hasBottomSide && !hasRightSide))
        {
            result.extend = true;
            return result;
        }
        // clang-format on
    }

    return result;
}

template <bool taAllowedToLoadAdjacent>
hg::PZInteger World::_calcOpennessAt(hg::PZInteger aX, hg::PZInteger aY) {
    if (_config.maxCellOpenness == 0) {
        return 0;
    }

    const auto maxRing = static_cast<hg::PZInteger>((_config.maxCellOpenness + 1) / 2);

    // Ring 0 = just the cell at (aX, aY)
    // Ring 1 =  8 cells around Ring 0
    // Ring 2 = 16 cells around Ring 1
    // and so on...
    for (hg::PZInteger ring = 0; ring < maxRing; ring += 1) {
        const auto ras = _assessRing<taAllowedToLoadAdjacent>(aX, aY, ring);

        if (!ras.hasOccupiedCells) {
            continue;
        }

        if (ring == 0) {
            return 0; // Ring 0 - special case
        }

        return ((ring * 2) - 1 + hg::ToPz(ras.extend));
    }

    return ((maxRing * 2) - 1);
}

///////////////////////////////////////////////////////////////////////////
// asdasd                                                                //
///////////////////////////////////////////////////////////////////////////

void World::update() {
    _chunkHolder.update();
}

void World::prune() {
    _chunkHolder.prune();
}

///////////////////////////////////////////////////////////////////////////
// CONVERSIONS                                                           //
///////////////////////////////////////////////////////////////////////////

hg::math::Vector2pz World::posToCell(double aX, double aY) const {
    HG_VALIDATE_ARGUMENT(aX >= 0.0 && aX < _config.cellCountX * _config.cellResolution);
    HG_VALIDATE_ARGUMENT(aY >= 0.0 && aY < _config.cellCountY * _config.cellResolution);
    return posToCellUnchecked(aX, aY);
}

hg::math::Vector2pz World::posToCell(hg::math::Vector2d aPos) const {
    HG_VALIDATE_ARGUMENT(aPos.x >= 0.0 && aPos.x < _config.cellCountX * _config.cellResolution);
    HG_VALIDATE_ARGUMENT(aPos.y >= 0.0 && aPos.y < _config.cellCountY * _config.cellResolution);
    return posToCellUnchecked(aPos);
}

hg::math::Vector2pz World::posToCellUnchecked(double aX, double aY) const {
    return {static_cast<hg::PZInteger>(aX / _config.cellResolution),
            static_cast<hg::PZInteger>(aY / _config.cellResolution)};
}

hg::math::Vector2pz World::posToCellUnchecked(hg::math::Vector2d aPos) const {
    return {static_cast<hg::PZInteger>(aPos.x / _config.cellResolution),
            static_cast<hg::PZInteger>(aPos.y / _config.cellResolution)};
}

ChunkId World::cellToChunkId(hg::PZInteger aX, hg::PZInteger aY) const {
    HG_VALIDATE_ARGUMENT(aX < _config.cellCountX);
    HG_VALIDATE_ARGUMENT(aY < _config.cellCountY);
    return cellToChunkIdUnchecked(aX, aY);
}

ChunkId World::cellToChunkId(hg::math::Vector2pz aCell) const {
    HG_VALIDATE_ARGUMENT(aCell.x < _config.cellCountX);
    HG_VALIDATE_ARGUMENT(aCell.y < _config.cellCountY);
    return cellToChunkIdUnchecked(aCell);
}

ChunkId World::cellToChunkIdUnchecked(hg::PZInteger aX, hg::PZInteger aY) const {
    return {aX / _config.cellsPerChunkX, aY / _config.cellsPerChunkY};
}

ChunkId World::cellToChunkIdUnchecked(hg::math::Vector2pz aCell) const {
    return {aCell.x / _config.cellsPerChunkX, aCell.y / _config.cellsPerChunkY};
}

///////////////////////////////////////////////////////////////////////////
// LOCKING                                                               //
///////////////////////////////////////////////////////////////////////////

World::EditPermission::~EditPermission() {
    // signal world
}

std::unique_ptr<World::EditPermission> World::getPermissionToEdit() {
    return std::unique_ptr<EditPermission>{new EditPermission{}}; // TODO: temporary
}

///////////////////////////////////////////////////////////////////////////
// MARK: CELL GETTERS                                                    //
///////////////////////////////////////////////////////////////////////////

double World::getCellResolution() const {
    return _config.cellResolution;
}

double World::getWallHeight() const {
    return _config.wallHeight;
}

hg::PZInteger World::getCellCountX() const {
    return _config.cellCountX;
}

hg::PZInteger World::getCellCountY() const {
    return _config.cellCountY;
}

///////////////////////////////////////////////////////////////////////////
// MARK: CELL UPDATERS                                                   //
///////////////////////////////////////////////////////////////////////////

void World::toggleGeneratorMode(const EditPermission&, bool aGeneratorMode) {
    if (aGeneratorMode) {
        HG_VALIDATE_PRECONDITION(_isInGeneratorMode == false);
        _isInGeneratorMode = true;
    } else {
        HG_VALIDATE_PRECONDITION(_isInGeneratorMode == true);
        _isInGeneratorMode = false;

        // When generator mode is toggled off, refresh all cells in currently loaded chunks
        auto       iter = availableChunksBegin();
        const auto end  = availableChunksEnd();
        for (; iter != end; ++iter) {
            const auto& chunkId = *iter;

            const hg::PZInteger startX = chunkId.x * _config.cellsPerChunkX;
            const hg::PZInteger startY = chunkId.y * _config.cellsPerChunkY;
            const hg::PZInteger endX   = startX + _config.cellsPerChunkX - 1;
            const hg::PZInteger endY   = startY + _config.cellsPerChunkY - 1;

            for (hg::PZInteger y = startY; y <= endY; y += 1) {
                for (hg::PZInteger x = startX; x <= endX; x += 1) {
                    _refreshCellAtUnchecked(x, y);
                }
            }
        }
    }
}

///////////////////////////////////////////////////////////////////////////
// MARK: CHUNKS                                                          //
///////////////////////////////////////////////////////////////////////////

const Chunk* World::getChunkAt(hg::PZInteger aX, hg::PZInteger aY) const {
    HG_VALIDATE_ARGUMENT(aX < getCellCountX());
    HG_VALIDATE_ARGUMENT(aY < getCellCountY());
    return getChunkAtUnchecked(aX, aY);
}

const Chunk* World::getChunkAtUnchecked(hg::PZInteger aX, hg::PZInteger aY) const {
    const auto id = ChunkId{static_cast<std::uint16_t>(aX / _config.cellsPerChunkX),
                            static_cast<std::uint16_t>(aY / _config.cellsPerChunkY)};
    return getChunkAtIdUnchecked(id);
}

const Chunk* World::getChunkAt(hg::math::Vector2pz aCell) const {
    return getChunkAt(aCell.x, aCell.y);
}

const Chunk* World::getChunkAtUnchecked(hg::math::Vector2pz aCell) const {
    return getChunkAtUnchecked(aCell.x, aCell.y);
}

const Chunk* World::getChunkAtId(ChunkId aChunkId) const {
    HG_VALIDATE_ARGUMENT(static_cast<hg::PZInteger>(aChunkId.x) < getChunkCountX());
    HG_VALIDATE_ARGUMENT(static_cast<hg::PZInteger>(aChunkId.y) < getChunkCountY());
    return getChunkAtIdUnchecked(aChunkId);
}

const Chunk* World::getChunkAtIdUnchecked(ChunkId aChunkId) const {
    return _chunkHolder.getChunkAtIdUnchecked(aChunkId);
}

const Chunk& World::getChunkAt(const EditPermission& aPerm, hg::PZInteger aX, hg::PZInteger aY) const {
    HG_VALIDATE_ARGUMENT(aX < getCellCountX());
    HG_VALIDATE_ARGUMENT(aY < getCellCountY());
    return getChunkAtUnchecked(aPerm, aX, aY);
}

const Chunk& World::getChunkAtUnchecked(const EditPermission& aPerm,
                                        hg::PZInteger         aX,
                                        hg::PZInteger         aY) const {
    const auto id = ChunkId{static_cast<std::uint16_t>(aX / _config.cellsPerChunkX),
                            static_cast<std::uint16_t>(aY / _config.cellsPerChunkY)};
    return getChunkAtIdUnchecked(aPerm, id);
}

const Chunk& World::getChunkAt(const EditPermission& aPerm, hg::math::Vector2pz aCell) const {
    return getChunkAt(aPerm, aCell.x, aCell.y);
}

const Chunk& World::getChunkAtUnchecked(const EditPermission& aPerm, hg::math::Vector2pz aCell) const {
    return getChunkAtUnchecked(aPerm, aCell.x, aCell.y);
}

const Chunk& World::getChunkAtId(const EditPermission& aPerm, ChunkId aChunkId) const {
    HG_VALIDATE_ARGUMENT(static_cast<hg::PZInteger>(aChunkId.x) < getChunkCountX());
    HG_VALIDATE_ARGUMENT(static_cast<hg::PZInteger>(aChunkId.y) < getChunkCountY());
    return getChunkAtIdUnchecked(aPerm, aChunkId);
}

const Chunk& World::getChunkAtIdUnchecked(const EditPermission& /*aPerm*/, ChunkId aChunkId) const {
    return _chunkHolder.getChunkAtIdUnchecked(aChunkId, detail::LOAD_IF_MISSING);
}

///////////////////////////////////////////////////////////////////////////
// MARK: ACTIVE AREAS                                                    //
///////////////////////////////////////////////////////////////////////////

ActiveArea World::createActiveArea() {
    return _chunkHolder.createNewActiveArea();
}

///////////////////////////////////////////////////////////////////////////
// MARK: PRIVATE METHODS                                                 //
///////////////////////////////////////////////////////////////////////////

// ===== Listeners =====

void World::_attachBinder(hg::NeverNull<Binder*> aBinder, std::int32_t aPriority) {
    const auto iter = std::find_if(_binders.begin(), _binders.end(), [aBinder](auto& aPair) {
        return (aPair.first == aBinder);
    });
    if (iter != _binders.end()) {
        HG_THROW_TRACED(hg::TracedLogicError,
                        0,
                        "Binder at address {:#x} already attached.",
                        reinterpret_cast<std::uintptr_t>(aBinder.get()));
    }

    _binders.push_back(std::make_pair(aBinder, aPriority));

    std::sort(_binders.begin(), _binders.end(), [](const auto& aLhs, const auto& aRhs) {
        return (aLhs.second < aRhs.second);
    });
}

// ===== Subcomponents =====

void World::_connectSubcomponents() {
    _chunkHolder.setBinder(this);
    _chunkHolder.setChunkSpooler(_chunkSpooler);
    _chunkDiskIoHandler->setBinder(this);
    _chunkSpooler->setDiskIoHandler(_chunkDiskIoHandler);
}

void World::_disconnectSubcomponents() {
    _chunkSpooler->setDiskIoHandler(nullptr);
    _chunkDiskIoHandler->setBinder(nullptr);
    _chunkHolder.setChunkSpooler(nullptr);
    _chunkHolder.setBinder(nullptr);
}

// ===== Callbacks =====

void World::_refreshCellsInAndAroundChunk(ChunkId aChunkId) {
    const hg::PZInteger top  = aChunkId.y * _config.cellsPerChunkY;
    const hg::PZInteger left = aChunkId.x * _config.cellsPerChunkX;

    const auto maxOffset = static_cast<hg::PZInteger>(_config.maxCellOpenness / 2);

    const auto startX = std::max<hg::PZInteger>(0, left - maxOffset);
    const auto startY = std::max<hg::PZInteger>(0, top - maxOffset);
    const auto endX =
        std::min<hg::PZInteger>(_config.cellCountX - 1, left + _config.cellsPerChunkX - 1 + maxOffset);
    const auto endY =
        std::min<hg::PZInteger>(_config.cellCountY - 1, top + _config.cellsPerChunkY - 1 + maxOffset);

    for (hg::PZInteger y = startY; y <= endY; y += 1) {
        for (hg::PZInteger x = startX; x <= endX; x += 1) {
            _refreshCellAtUnchecked(x, y);
        }
    }
}

void World::onChunkReady(ChunkId aChunkId) {
    for (const auto& [binder, priority] : _binders) {
        (void)priority;
        binder->onChunkReady(aChunkId);
    }
}

void World::willIntegrateNewChunk(ChunkId                      aId,
                                  Chunk&                       aChunk,
                                  const ChunkMemoryLayoutInfo& aChunkMemLayout) {
    for (const auto& [binder, priority] : _binders) {
        (void)priority;
        binder->willIntegrateNewChunk(aId, aChunk, aChunkMemLayout);
    }
}

void World::willIntegrateLoadedChunk(ChunkId                      aId,
                                     Chunk&                       aChunk,
                                     const ChunkMemoryLayoutInfo& aChunkMemLayout) {
    for (const auto& [binder, priority] : _binders) {
        (void)priority;
        binder->willIntegrateLoadedChunk(aId, aChunk, aChunkMemLayout);
    }
}

void World::didIntegrateChunk(ChunkId                      aId,
                              const Chunk&                 aChunk,
                              const ChunkMemoryLayoutInfo& aChunkMemLayout) {
    _refreshCellsInAndAroundChunk(aId);

    for (const auto& [binder, priority] : _binders) {
        (void)priority;
        binder->didIntegrateChunk(aId, aChunk, aChunkMemLayout);
    }
}

void World::willSeparateChunk(ChunkId                      aId,
                              const Chunk&                 aChunk,
                              const ChunkMemoryLayoutInfo& aChunkMemLayout) {
    for (const auto& [binder, priority] : _binders) {
        (void)priority;
        binder->willSeparateChunk(aId, aChunk, aChunkMemLayout);
    }
}

void World::didSeparateChunk(ChunkId aId, Chunk& aChunk, const ChunkMemoryLayoutInfo& aChunkMemLayout) {
    // No need to refresh cell after a chunk is separated; cells near the edges of the loaded parts
    // of the world can have slightly inaccurate information - it doesn't matter.

    for (const auto& [binder, priority] : _binders) {
        (void)priority;
        binder->didSeparateChunk(aId, aChunk, aChunkMemLayout);
    }
}

std::unique_ptr<ChunkExtensionInterface> World::createChunkExtension() {
    for (const auto& [binder, priority] : _binders) {
        (void)priority;
        auto extension = binder->createChunkExtension();
        if (extension != nullptr) {
            return extension;
        }
    }
    return nullptr;
}

// ===== Editing cells =====

void World::_startEdit() {
    HG_VALIDATE_PRECONDITION(_isInEditMode == false);

    _isInEditMode = true;

    _editMinX = -1;
    _editMinY = -1;
    _editMaxX = -1;
    _editMaxY = -1;

    _cellEditInfos.clear();
}

void World::_endEdit() {
    if (!_isInGeneratorMode) {
        if (_editMinX != -1 && _editMinY != -1 && _editMaxX != -1 && _editMaxY != -1) {
            const auto maxOffset = static_cast<hg::PZInteger>(_config.maxCellOpenness / 2);

            const auto startX = std::max<hg::PZInteger>(0, _editMinX - maxOffset);
            const auto startY = std::max<hg::PZInteger>(0, _editMinY - maxOffset);
            const auto endX   = std::min<hg::PZInteger>(_config.cellCountX - 1, _editMaxX + maxOffset);
            const auto endY   = std::min<hg::PZInteger>(_config.cellCountY - 1, _editMaxY + maxOffset);

            for (hg::PZInteger y = startY; y <= endY; y += 1) {
                for (hg::PZInteger x = startX; x <= endX; x += 1) {
                    _refreshCellAtUnchecked(x, y);
                }
            }
        }
    }

    for (const auto& [binder, priority] : _binders) {
        binder->onCellsEdited(_cellEditInfos);
    }

    _isInEditMode = false;
}

void World::_refreshCellAtUnchecked(hg::PZInteger aX, hg::PZInteger aY) {
    const auto chunkX = aX / _config.cellsPerChunkX;
    const auto chunkY = aY / _config.cellsPerChunkY;

    auto* chunk = _chunkHolder.getChunkAtIdUnchecked({chunkX, chunkY});

    if (chunk) {
        cell::SpatialInfo spatialInfo;
        chunk->getCellDataAtUnchecked(getChunkMemoryLayoutInfo(),
                                      aX % _config.cellsPerChunkX,
                                      aY % _config.cellsPerChunkY,
                                      &spatialInfo);

        spatialInfo.openness = static_cast<std::uint8_t>(_calcOpennessAt<false>(aX, aY));
        spatialInfo.obFlags  = [this, aX, aY, chunk, spatialInfo]() -> std::uint8_t {
            if (spatialInfo.openness > 2) {
                return 0;
            }

            std::uint8_t res = 0;

            auto getNeighborObstruction = [this](hg::PZInteger aX,
                                                 hg::PZInteger aY,
                                                 std::uint8_t  aRelevantFlags) -> std::uint8_t //
            {
                if (const auto si = _getSpatialInfoOfCellAtUnchecked<false>(aX, aY); si) {
                    return si->hasNonEmptyWallShape()
                                ? (SHAPE_OBSTRUCTION_FLAGS[hg::ToSz(si->wallShape)] & aRelevantFlags)
                                : 0;
                } else {
                    return aRelevantFlags;
                }
            };

            res |= (aY <= 0)
                        ? cell::OBSTRUCTED_FULLY_BY_NORTH_NEIGHBOR
                        : getNeighborObstruction(aX, aY - 1, OBSTRUCTS_SOUTH | OBSTRUCTS_SOUTH_FULLY);

            res |= (aX <= 0) ? cell::OBSTRUCTED_FULLY_BY_WEST_NEIGHBOR
                              : getNeighborObstruction(aX - 1, aY, OBSTRUCTS_EAST | OBSTRUCTS_EAST_FULLY);

            res |= (aX >= getCellCountX() - 1)
                        ? cell::OBSTRUCTED_FULLY_BY_EAST_NEIGHBOR
                        : getNeighborObstruction(aX + 1, aY, OBSTRUCTS_WEST | OBSTRUCTS_WEST_FULLY);

            res |= (aY >= getCellCountY() - 1)
                        ? cell::OBSTRUCTED_FULLY_BY_SOUTH_NEIGHBOR
                        : getNeighborObstruction(aX, aY + 1, OBSTRUCTS_NORTH | OBSTRUCTS_NORTH_FULLY);

            return res;
        }();

        chunk->setCellDataAtUnchecked(getChunkMemoryLayoutInfo(),
                                      aX % _config.cellsPerChunkX,
                                      aY % _config.cellsPerChunkY,
                                      &spatialInfo);
    }
}

void World::_setCellDataAtUnchecked(Chunk&                   aChunk,
                                    hg::PZInteger            aX,
                                    hg::PZInteger            aY,
                                    const cell::SpatialInfo* aSpatialInfo) {
    const auto cellRelativeX = aX % _config.cellsPerChunkX;
    const auto cellRelativeY = aY % _config.cellsPerChunkY;

    cell::SpatialInfo currentSi;
    aChunk.getCellDataAtUnchecked(getChunkMemoryLayoutInfo(), cellRelativeX, cellRelativeY, &currentSi);

    if (currentSi.wallShape == aSpatialInfo->wallShape) {
        // In this case, there is no refresh needed because the walls are of the same shape,
        // or are both non-existent
        goto SWAP_WALL;
    }

    if (_editMinX == -1 || _editMaxX == -1) {
        _editMinX = _editMaxX = aX;
    } else {
        _editMinX = std::min(_editMinX, aX);
        _editMaxX = std::max(_editMaxX, aX);
    }

    if (_editMinY == -1 || _editMaxY == -1) {
        _editMinY = _editMaxY = aY;
    } else {
        _editMinY = std::min(_editMinY, aY);
        _editMaxY = std::max(_editMaxY, aY);
    }

SWAP_WALL:
    aChunk.setCellDataAtUnchecked(getChunkMemoryLayoutInfo(),
                                  cellRelativeX,
                                  cellRelativeY,
                                  aSpatialInfo);

    // clang-format off
    _cellEditInfos.push_back({{aX, aY}, Binder::CellEditInfo::WALL}); // !!!!!!!!!!!! SKIP IN GENERATOR MODE !!!!!!!!!!!!!!!!
    if (_isInGeneratorMode) {
        prune();
    }
    // clang-format on
}

} // namespace gridgoblin
} // namespace jbatnozic
