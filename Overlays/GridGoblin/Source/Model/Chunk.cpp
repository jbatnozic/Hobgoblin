// Copyright 2024 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#include <GridGoblin/Model/Chunk.hpp>

#include <GridGoblin/Model/Chunk_extension.hpp>

#include <Hobgoblin/HGExcept.hpp>

namespace jbatnozic {
namespace gridgoblin {

// void Chunk::setAll(const CellModel& aCell) {
//     for (hg::PZInteger y = 0; y < getCellCountY(); y += 1) {
//         for (hg::PZInteger x = 0; x < getCellCountX(); x += 1) {
//             _cells[y][x] = detail::CellModelExt{aCell};
//         }
//     }
// }

Chunk::Chunk(const ChunkMemoryLayoutInfo& aMemLayout) {
    _impl.init(reinterpret_cast<const detail::ChunkImpl::MemoryLayoutInfo&>(aMemLayout));
}

void Chunk::setExtension(std::unique_ptr<ChunkExtensionInterface> aChunkExtension) {
    _impl.setExtension(std::move(aChunkExtension));
}

std::unique_ptr<ChunkExtensionInterface> Chunk::releaseExtension() {
    return _impl.releaseExtension();
}

///////////////////////////////////////////////////////////////////////////
// PRIVATE                                                               //
///////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////
// FREE FUNCTIONS                                                        //
///////////////////////////////////////////////////////////////////////////

namespace detail {
// bool AreCellsEqual(const Chunk& aChunk1, const Chunk& aChunk2) {
//     if (aChunk1.getCellCountX() != aChunk2.getCellCountX() ||
//         aChunk1.getCellCountY() != aChunk2.getCellCountY()) {
//         return false;
//     }

//     for (hg::PZInteger y = 0; y < aChunk1.getCellCountY(); y += 1) {
//         for (hg::PZInteger x = 0; x < aChunk2.getCellCountX(); x += 1) {
//             const auto& lhsCell = static_cast<const CellModel&>(aChunk1.getCellAtUnchecked(x, y));
//             const auto& rhsCell = static_cast<const CellModel&>(aChunk2.getCellAtUnchecked(x, y));
//             if (lhsCell != rhsCell) {
//                 return false;
//             }
//         }
//     }

//     return true;
// }
} // namespace detail

} // namespace gridgoblin
} // namespace jbatnozic
