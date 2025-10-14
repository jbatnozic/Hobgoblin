// Copyright 2024 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#pragma once

#include <Hobgoblin/Common.hpp>

#include <GridGoblin/Model/Cell.hpp>
#include <GridGoblin/Model/Chunk.hpp>
#include <GridGoblin/Model/Chunk_extension.hpp>
#include <GridGoblin/Private/Chunk_impl.hpp>

#include <Hobgoblin/HGExcept.hpp>
#include <Hobgoblin/Utility/Packet.hpp>

#include <rapidjson/document.h>

#include <functional>
#include <memory>
#include <string>

namespace jbatnozic {
namespace gridgoblin {
namespace detail {

///////////////////////////////////////////////////////////////////////////
// JSON CONVERSIONS (FOR PERSISTENT STORAGE)                             //
///////////////////////////////////////////////////////////////////////////

namespace json = rapidjson;

class JsonParseError : public jbatnozic::hobgoblin::TracedRuntimeError {
public:
    using jbatnozic::hobgoblin::TracedRuntimeError::TracedRuntimeError;
};

using ChunkExtensionFactory = std::function<std::unique_ptr<ChunkExtensionInterface>(const Chunk&)>;

struct ReusableConversionBuffers;

hg::NeverNull<ReusableConversionBuffers*> NewReusableConversionBuffers();
void                                      DeleteReusableConversionBuffers(ReusableConversionBuffers*);

// MARK: Cell

using JsonAllocator = decltype(std::declval<json::Document>().GetAllocator());

json::Value CellKindIdToJson(cell::CellKindId aCellKindId, JsonAllocator& aAllocator);
json::Value FloorSpriteToJson(cell::FloorSprite aFloorSprite, JsonAllocator& aAllocator);
json::Value WallSpriteToJson(cell::WallSprite aWallSprite, JsonAllocator& aAllocator);
json::Value SpatialInfoToJson(cell::SpatialInfo aSpatialInfo, JsonAllocator& aAllocator);
json::Value RendererAuxDataToJson(cell::RendererAuxData aRendererAuxData, JsonAllocator& aAllocator);
json::Value UserDataToJson(cell::UserData aUserData, JsonAllocator& aAllocator);

cell::CellKindId      JsonToCellKindId(const json::Value& aJsonValue);
cell::FloorSprite     JsonToFloorSprite(const json::Value& aJsonValue);
cell::WallSprite      JsonToWallSprite(const json::Value& aJsonValue);
cell::SpatialInfo     JsonToSpatialInfo(const json::Value& aJsonValue);
cell::RendererAuxData JsonToRendererAuxData(const json::Value& aJsonValue);
cell::UserData        JsonToUserData(const json::Value& aJsonValue);

// //! Creates a JSON Value from the passed CellModel instance using the allocator
// //! of the passed document (does not attach the Value anywhere).
// //!
// //! \note Any data the cell might be extended with will NOT be included.
// json::Value CellToJson(const CellModel& aCell, json::Document& aJsonDocument);

// //! Creates a CellModel instance from the passed JSON Value.
// //!
// //! \throws JsonParseError is the JSON data does not correspond to the expected format.
// CellModel JsonToCell(const json::Value& aJsonValue);

// MARK: Chunk

//! Creates a JSON Document from the passed Chunk.
//!
//! \note Any data the chunk's cells might be extended with will NOT be included.
json::Document ChunkToJson(const Chunk&                 aChunk,
                           BuildingBlockMask            aBuildingBlocks,
                           const ChunkMemoryLayoutInfo& aChunkMemLayout,
                           ReusableConversionBuffers*   aReusableConversionBuffers = nullptr);

//! Creates a Chunk from the passed JSON Document.
//!
//! \throws JsonParseError is the JSON data does not correspond to the expected format.
Chunk JsonToChunk(const json::Document&        aJsonDocument,
                  const ChunkMemoryLayoutInfo& aChunkMemLayout,
                  const ChunkExtensionFactory& aChunkExtensionFactory     = {},
                  ReusableConversionBuffers*   aReusableConversionBuffers = nullptr);

//! Creates a JSON-encoded string (without whitespaces, newlines or any pretty
//! formatting) from the passed Chunk.
//!
//! \note Any data the chunk's cells might be extended with will NOT be included.
std::string ChunkToJsonString(const Chunk&                 aChunk,
                              BuildingBlockMask            aBuildingBlocks,
                              const ChunkMemoryLayoutInfo& aChunkMemLayout,
                              ReusableConversionBuffers*   aReusableConversionBuffers = nullptr);

//! Creates a Chunk from the passed JSON-encoded string.
//!
//! \throws JsonParseError is the JSON data does not correspond to the expected format.
Chunk JsonStringToChunk(std::string                  aJsonString,
                        const ChunkMemoryLayoutInfo& aChunkMemLayout,
                        const ChunkExtensionFactory& aChunkExtensionFactory     = {},
                        ReusableConversionBuffers*   aReusableConversionBuffers = nullptr);

} // namespace detail
} // namespace gridgoblin
} // namespace jbatnozic
