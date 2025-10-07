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
    const auto totalByteSize = detail::ChunkMemoryLayoutInfoAccessor::getTotalSize(aMemLayout);

    _mem = (std::byte*)hg::AlignedAlloc(alignof(void*), totalByteSize);
    HG_HARD_ASSERT(_mem != nullptr);

    std::memset(_mem, 0x00, totalByteSize); // TODO: temp!!!!!!!!!!!!!!!!!!!

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
