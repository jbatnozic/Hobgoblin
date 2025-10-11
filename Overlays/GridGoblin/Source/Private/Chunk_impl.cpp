// Copyright 2024 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#include <Hobgoblin/Common/Aligned_alloc.hpp>
#include <Hobgoblin/HGExcept.hpp>

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

#define acc ChunkMemoryLayoutInfoAccessor

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

void ChunkImpl::alloc(const MemoryLayoutInfo& aMemLayout) {
    const auto totalByteSize = acc::getTotalSize(aMemLayout);

    _mem = (std::byte*)hg::AlignedAlloc(alignof(void*), totalByteSize);
    HG_HARD_ASSERT(_mem != nullptr);

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

void ChunkImpl::zeroOut(const MemoryLayoutInfo& aMemLayout) {
    HG_VALIDATE_PRECONDITION(_mem != nullptr);

    const auto totalByteSize = acc::getTotalSize(aMemLayout);
    std::memset(_mem + sizeof(ChunkExtensionInterface*),
                0x00,
                totalByteSize - sizeof(ChunkExtensionInterface*));
}

void ChunkImpl::setAll(const MemoryLayoutInfo& aMemLayout, const FatCell& aCell) {
    HG_VALIDATE_PRECONDITION(_mem != nullptr);

    const auto width  = acc::getChunkWidth(aMemLayout);
    const auto height = acc::getChunkHeight(aMemLayout);

    if (auto offset = acc::getCellKindIdOffset(aMemLayout); offset != acc::INVALID_OFFSET) {
        for (hg::PZInteger y = 0; y < height; ++y) {
            for (hg::PZInteger x = 0; x < width; ++x) {
                getCellKindIdAtUnchecked(aMemLayout, {x, y}) = aCell.cellKindId;
            }
        }
    }

    if (auto offset = acc::getFloorSpriteOffset(aMemLayout); offset != acc::INVALID_OFFSET) {
        for (hg::PZInteger y = 0; y < height; ++y) {
            for (hg::PZInteger x = 0; x < width; ++x) {
                getFloorSpriteAtUnchecked(aMemLayout, {x, y}) = aCell.floorSprite;
            }
        }
    }

    if (auto offset = acc::getWallSpriteOffset(aMemLayout); offset != acc::INVALID_OFFSET) {
        for (hg::PZInteger y = 0; y < height; ++y) {
            for (hg::PZInteger x = 0; x < width; ++x) {
                getWallSpriteAtUnchecked(aMemLayout, {x, y}) = aCell.wallSprite;
            }
        }
    }

    if (auto offset = acc::getSpatialInfoOffset(aMemLayout); offset != acc::INVALID_OFFSET) {
        for (hg::PZInteger y = 0; y < height; ++y) {
            for (hg::PZInteger x = 0; x < width; ++x) {
                getSpatialInfoAtUnchecked(aMemLayout, {x, y}) = aCell.spatialInfo;
            }
        }
    }

    if (auto offset = acc::getRendererAuxDataOffset(aMemLayout); offset != acc::INVALID_OFFSET) {
        for (hg::PZInteger y = 0; y < height; ++y) {
            for (hg::PZInteger x = 0; x < width; ++x) {
                getRendererAuxDataAtUnchecked(aMemLayout, {x, y}) = aCell.rendererAuxData;
            }
        }
    }

    if (auto offset = acc::getUserDataOffset(aMemLayout); offset != acc::INVALID_OFFSET) {
        for (hg::PZInteger y = 0; y < height; ++y) {
            for (hg::PZInteger x = 0; x < width; ++x) {
                getUserDataAtUnchecked(aMemLayout, {x, y}) = aCell.userData;
            }
        }
    }
}

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
