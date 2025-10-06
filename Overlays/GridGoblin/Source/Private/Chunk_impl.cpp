// Copyright 2024 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#include <Hobgoblin/Common/Aligned_alloc.hpp>
#include <Hobgoblin/HGExcept.hpp>
#include <Hobgoblin/Math/Core.hpp>

#include <GridGoblin/Model/Chunk_extension.hpp>
#include <GridGoblin/Private/Chunk_impl.hpp>

namespace jbatnozic {
namespace gridgoblin {

namespace hg = ::jbatnozic::hobgoblin;

namespace {
void IncUntilAligned(std::size_t& aByteCounter, std::size_t aAlignment) {
    aByteCounter = hg::math::IntegralCeilDiv(aByteCounter, aAlignment) * aAlignment;
}
} // namespace

namespace detail {

ChunkImpl::MemoryLayoutInfo ChunkImpl::calcMemoryLayoutInfo(hg::PZInteger     aChunkWidth,
                                                            hg::PZInteger     aChunkHeight,
                                                            BuildingBlockMask aBuildingBlocks) {
    MemoryLayoutInfo meminfo{.chunkWidth = aChunkWidth, .chunkHeight = aChunkHeight};

    const auto cellCount = static_cast<std::size_t>(aChunkWidth * aChunkHeight);

    std::size_t byteCounter = 0;

    // Extension
    {
        // IMPORTANT: Extension pointer has to always be at offset zero, so that it can be cleaned
        //            up in the destructor without having access to a `MemoryLayoutInfo` object.
        // meminfo.offset.extension = byteCounter;
        byteCounter += 1 * sizeof(ChunkExtensionInterface*);
    }

    // Cell kind ID
    if ((aBuildingBlocks & BuildingBlock::CELL_KIND_ID) != BuildingBlock::NONE) {
        IncUntilAligned(byteCounter, alignof(cell::CellKindId));
        meminfo.offset.cellKindId = byteCounter;
        byteCounter += cellCount * sizeof(cell::CellKindId);
    }

    // Floor sprite
    if ((aBuildingBlocks & BuildingBlock::FLOOR_SPRITE) != BuildingBlock::NONE) {
        IncUntilAligned(byteCounter, alignof(cell::FloorSprite));
        meminfo.offset.floorSprite = byteCounter;
        byteCounter += cellCount * sizeof(cell::FloorSprite);
    }

    // Wall sprite
    if ((aBuildingBlocks & BuildingBlock::WALL_SPRITE) != BuildingBlock::NONE) {
        IncUntilAligned(byteCounter, alignof(cell::WallSprite));
        meminfo.offset.wallSprite = byteCounter;
        byteCounter += cellCount * sizeof(cell::WallSprite);
    }

    // Spatial info
    if ((aBuildingBlocks & BuildingBlock::SPATIAL_INFO) != BuildingBlock::NONE) {
        IncUntilAligned(byteCounter, alignof(cell::SpatialInfo));
        meminfo.offset.spatialInfo = byteCounter;
        byteCounter += cellCount * sizeof(cell::SpatialInfo);
    }

    // Renderer aux data
    if ((aBuildingBlocks & BuildingBlock::RENDERER_AUX_DATA) != BuildingBlock::NONE) {
        IncUntilAligned(byteCounter, alignof(cell::RendererAuxData));
        meminfo.offset.rendererAuxData = byteCounter;
        byteCounter += cellCount * sizeof(cell::RendererAuxData);
    }

    // User data
    if ((aBuildingBlocks & BuildingBlock::USER_DATA) != BuildingBlock::NONE) {
        IncUntilAligned(byteCounter, alignof(cell::UserData));
        meminfo.offset.userData = byteCounter;
        byteCounter += cellCount * sizeof(cell::UserData);
    }

    meminfo.totalSize = byteCounter;

    return meminfo;
}

ChunkImpl::ChunkImpl(ChunkImpl&& aOther)
    : _mem{aOther._mem} {
    aOther._mem = nullptr;
}

ChunkImpl& ChunkImpl::operator=(ChunkImpl&& aOther) {
    if (&aOther != this) {
        makeEmpty();
        std::swap(_mem, aOther._mem);
    }
    return *this;
}

ChunkImpl::~ChunkImpl() {
    makeEmpty();
}

void ChunkImpl::init(const MemoryLayoutInfo& aMemLayout) {
    _mem = (std::byte*)hg::AlignedAlloc(alignof(void*), aMemLayout.totalSize);
    HG_HARD_ASSERT(_mem != nullptr);

    std::memset(_mem, 0x00, aMemLayout.totalSize); // TODO: temp!!!!!!!!!!!!!!!!!!!

    _storeChunkExtensionPointer(nullptr);
}

void ChunkImpl::makeEmpty() {
    if (!isEmpty()) {
        releaseExtension().reset();
        hg::AlignedFree(_mem);
        _mem = nullptr;
    }
}

///////////////////////////////////////////////////////////////////////////
// MARK: CELL DATA                                                       //
///////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////
// MARK: EXTENSION                                                       //
///////////////////////////////////////////////////////////////////////////

void ChunkImpl::setExtension(std::unique_ptr<ChunkExtensionInterface> aChunkExtension) {
    releaseExtension().reset();
    _storeChunkExtensionPointer(aChunkExtension.release());
}

ChunkExtensionInterface* ChunkImpl::getExtension() const {
    return _loadChunkExtensionPointer();
}

std::unique_ptr<ChunkExtensionInterface> ChunkImpl::releaseExtension() {
    std::unique_ptr<ChunkExtensionInterface> result{_loadChunkExtensionPointer()};
    _storeChunkExtensionPointer(nullptr);

    return result;
}

///////////////////////////////////////////////////////////////////////////
// MARK: PRIVATE                                                         //
///////////////////////////////////////////////////////////////////////////

namespace {
constexpr std::size_t EXTENSION_OFFSET = 0;
} // namespace

void ChunkImpl::_storeChunkExtensionPointer(ChunkExtensionInterface* aChunkExtensionPointer) {
    HG_ASSERT(!isEmpty());

    // The 'array' always has exactly one element (max one extension per chunk)
    auto* array = reinterpret_cast<ChunkExtensionInterface**>(_mem + EXTENSION_OFFSET);
    array[0]    = aChunkExtensionPointer;
}

ChunkExtensionInterface* ChunkImpl::_loadChunkExtensionPointer() const {
    HG_ASSERT(!isEmpty());

    // The 'array' always has exactly one element (max one extension per chunk)
    auto* array = reinterpret_cast<ChunkExtensionInterface**>(_mem + EXTENSION_OFFSET);
    return array[0];
}

} // namespace detail

} // namespace gridgoblin
} // namespace jbatnozic
