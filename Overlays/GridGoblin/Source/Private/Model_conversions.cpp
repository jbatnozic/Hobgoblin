// Copyright 2024 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#include <GridGoblin/Model/Chunk.hpp>
#include <GridGoblin/Private/Model_conversions.hpp>

#include <Hobgoblin/Common.hpp>
#include <Hobgoblin/HGExcept.hpp>
#include <Hobgoblin/Logging.hpp>
#include <Hobgoblin/Utility/Base64.hpp>
#include <Hobgoblin/Utility/Stream.hpp>

#include <rapidjson/document.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/stringbuffer.h>

#include <optional>
#include <string>
#include <type_traits>

namespace jbatnozic {
namespace gridgoblin {
namespace detail {

namespace json = rapidjson;

struct ReusableConversionBuffers {
    hg::util::BufferStream stream; //!< For extension serialization
    std::string            string;
};

hg::NeverNull<ReusableConversionBuffers*> NewReusableConversionBuffers() {
    return new ReusableConversionBuffers;
}

void DeleteReusableConversionBuffers(ReusableConversionBuffers* aBuffers) {
    delete aBuffers;
}

namespace {
constexpr auto LOG_ID = "GridGoblin";

const std::string TAG_EMPTY     = "empty";
const std::string TAG_BINSTREAM = "binary_stream";

#define ENSURE_JSON_CONTAINS(_value_, _member_name_, _member_type_)                                 \
    do {                                                                                            \
        if (!(_value_).HasMember(_member_name_) || !(_value_)[_member_name_].Is##_member_type_()) { \
            HG_THROW_TRACED(JsonParseError, 0, "No member named '{}' found.", _member_name_);       \
        }                                                                                           \
    } while (false)

template <class taIntegral>
taIntegral GetIntMember(const json::Value& aJsonValue, const char* aMemberName) {
    static_assert(std::is_integral<taIntegral>::value, "GetIntMember only works with integral types.");

    using T = std::remove_cvref_t<taIntegral>;

    if constexpr (std::is_same_v<T, std::int8_t> || std::is_same_v<T, std::int16_t> ||
                  std::is_same_v<T, std::int32_t>) {
        ENSURE_JSON_CONTAINS(aJsonValue, aMemberName, Int);
        return static_cast<taIntegral>(aJsonValue[aMemberName].GetInt());
    }

    if constexpr (std::is_same_v<T, std::uint8_t> || std::is_same_v<T, std::uint16_t> ||
                  std::is_same_v<T, std::uint32_t>) {
        ENSURE_JSON_CONTAINS(aJsonValue, aMemberName, Uint);
        return static_cast<taIntegral>(aJsonValue[aMemberName].GetUint());
    }

    if constexpr (std::is_same_v<T, std::int64_t>) {
        ENSURE_JSON_CONTAINS(aJsonValue, aMemberName, Int64);
        return static_cast<taIntegral>(aJsonValue[aMemberName].GetInt64());
    }

    if constexpr (std::is_same_v<T, std::uint64_t>) {
        ENSURE_JSON_CONTAINS(aJsonValue, aMemberName, Uint64);
        return static_cast<taIntegral>(aJsonValue[aMemberName].GetUint64());
    }

    HG_NOT_IMPLEMENTED("Type '" + std::string{hg::GetNameOfType<taIntegral>()} +
                       "' is not supported by GetIntMember.");
}

using ExtensionKind = std::optional<ChunkExtensionInterface::SerializationMethod>;

const std::string& ExtensionKindToString(const ExtensionKind& aExtensionKind) {
    if (!aExtensionKind.has_value() || *aExtensionKind == ExtensionKind::value_type::NONE) {
        return TAG_EMPTY;
    }

    switch (*aExtensionKind) {
    case ExtensionKind::value_type::BINARY_STREAM:
        return TAG_BINSTREAM;
        break;

    default:
        HG_UNREACHABLE("Invalid value for ExtensionKind ({}).", (int)*aExtensionKind);
    }
}

ExtensionKind StringToExtensionKind(const std::string& aString) {
    /*--*/ if (aString == TAG_EMPTY) {
        return std::nullopt;
    } else if (aString == TAG_BINSTREAM) {
        return {ExtensionKind::value_type::BINARY_STREAM};
    } else {
        HG_THROW_TRACED(JsonParseError, 0, "Unsupported string: '{}'.", aString);
    }
}

void Base64Encode(
    /*  in */ const ChunkExtensionInterface&               aChunkExtension,
    /*  in */ ChunkExtensionInterface::SerializationMethod aPreferredSerializationMethod,
    /*  in */ hg::util::BufferStream&                      aReusableBufStream,
    /* out */ std::string&                                 aBase64EncodeBuffer)
//
{
    aReusableBufStream.clear();
    aBase64EncodeBuffer.clear();

    switch (aPreferredSerializationMethod) {
    case ChunkExtensionInterface::SerializationMethod::BINARY_STREAM:
        {
            aChunkExtension.serialize(aReusableBufStream);

            const auto encodeSizePrediction = hg::util::GetRecommendedOutputBufferSizeForBase64Encode(
                aReusableBufStream.getDataSize());

            aBase64EncodeBuffer.resize(hg::pztos(encodeSizePrediction));

            const auto encodeSizeActual = hg::util::Base64Encode(aReusableBufStream.getData(),
                                                                 aReusableBufStream.getDataSize(),
                                                                 aBase64EncodeBuffer.data(),
                                                                 hg::stopz(aBase64EncodeBuffer.size()));

            aBase64EncodeBuffer.resize(hg::pztos(encodeSizeActual));
        }
        break;

    default:
        HG_UNREACHABLE("Invalid value for ChunkExtensionInterface::SerializationMethod ({}).",
                       (int)aPreferredSerializationMethod);
    }
}

void Base64Decode(
    /* out */ ChunkExtensionInterface&                     aChunkExtension,
    /*  in */ ChunkExtensionInterface::SerializationMethod aPreferredSerializationMethod,
    /*  in */ const char*                                  aBase64EncodedString,
    /*  in */ hg::PZInteger                                aBase64EncodedStringLength,
    /*  in */ std::string&                                 aReusableString)
//
{
    switch (aPreferredSerializationMethod) {
    case ChunkExtensionInterface::SerializationMethod::BINARY_STREAM:
        {
            const auto decodeSizePrediction =
                hg::util::GetRecommendedOutputBufferSizeForBase64Decode(aBase64EncodedStringLength);

            aReusableString.clear();
            aReusableString.resize(hg::pztos(decodeSizePrediction));

            const auto decodeSizeActual = hg::util::Base64Decode(aBase64EncodedString,
                                                                 aBase64EncodedStringLength,
                                                                 aReusableString.data(),
                                                                 hg::stopz(aReusableString.size()));

            hg::util::ViewStream vstream{aReusableString.data(), decodeSizeActual};

            aChunkExtension.deserialize(vstream);
        }
        break;

    default:
        HG_UNREACHABLE("Invalid value for ChunkExtensionInterface::SerializationMethod ({}).",
                       (int)aPreferredSerializationMethod);
    }
}
} // namespace

// MARK: Cell <-> JSON

json::Value CellKindIdToJson(cell::CellKindId aCellKindId, JsonAllocator& aAllocator) {
    json::Value value;
    value.SetObject();

    value.AddMember("value", json::Value{aCellKindId.value}.Move(), aAllocator);

    return value;
}

json::Value FloorSpriteToJson(cell::FloorSprite aFloorSprite, JsonAllocator& aAllocator) {
    json::Value value;
    value.SetObject();

    value.AddMember("id", json::Value{aFloorSprite.id}.Move(), aAllocator);

    return value;
}

json::Value WallSpriteToJson(cell::WallSprite aWallSprite, JsonAllocator& aAllocator) {
    json::Value value;
    value.SetObject();

    value.AddMember("id", json::Value{aWallSprite.id}.Move(), aAllocator);
    value.AddMember("id_reduced", json::Value{aWallSprite.id_reduced}.Move(), aAllocator);

    return value;
}

json::Value SpatialInfoToJson(cell::SpatialInfo aSpatialInfo, JsonAllocator& aAllocator) {
    json::Value value;
    value.SetObject();

    value.AddMember("wallShape",
                    json::Value{ShapeToString(aSpatialInfo.wallShape), aAllocator}.Move(),
                    aAllocator);
    value.AddMember("obFlags", json::Value{aSpatialInfo.obFlags}.Move(), aAllocator);
    value.AddMember("openness", json::Value{aSpatialInfo.openness}.Move(), aAllocator);

    return value;
}

json::Value RendererAuxDataToJson(cell::RendererAuxData aRendererAuxData, JsonAllocator& aAllocator) {
    json::Value value;
    value.SetObject();

    value.AddMember("mask", json::Value{aRendererAuxData.mask}.Move(), aAllocator);
    value.AddMember("mask2", json::Value{aRendererAuxData.mask2}.Move(), aAllocator);

    return value;
}

json::Value UserDataToJson(cell::UserData aUserData, JsonAllocator& aAllocator) {
    json::Value value;
    value.SetObject();

    value.AddMember("i64", json::Value{aUserData.i64}.Move(), aAllocator);

    return value;
}

cell::CellKindId JsonToCellKindId(const json::Value& aJsonValue) {
    cell::CellKindId result;
    result.value = GetIntMember<decltype(result.value)>(aJsonValue, "value");
    return result;
}

cell::FloorSprite JsonToFloorSprite(const json::Value& aJsonValue) {
    cell::FloorSprite result;
    result.id = GetIntMember<decltype(result.id)>(aJsonValue, "id");
    return result;
}

cell::WallSprite JsonToWallSprite(const json::Value& aJsonValue) {
    cell::WallSprite result;
    result.id         = GetIntMember<decltype(result.id)>(aJsonValue, "id");
    result.id_reduced = GetIntMember<decltype(result.id_reduced)>(aJsonValue, "id_reduced");
    return result;
}

cell::SpatialInfo JsonToSpatialInfo(const json::Value& aJsonValue) {
    cell::SpatialInfo result;
    result.wallShape = StringToShape(aJsonValue["wallShape"].GetString());
    result.obFlags   = GetIntMember<decltype(result.obFlags)>(aJsonValue, "obFlags");
    result.openness  = GetIntMember<decltype(result.openness)>(aJsonValue, "openness");
    return result;
}

cell::RendererAuxData JsonToRendererAuxData(const json::Value& aJsonValue) {
    cell::RendererAuxData result;
    result.mask  = GetIntMember<decltype(result.mask)>(aJsonValue, "mask");
    result.mask2 = GetIntMember<decltype(result.mask2)>(aJsonValue, "mask2");
    return result;
}

cell::UserData JsonToUserData(const json::Value& aJsonValue) {
    cell::UserData result;
    result.i64 = GetIntMember<decltype(result.i64)>(aJsonValue, "i64");
    return result;
}

// MARK: Chunk <-> JSON

json::Document ChunkToJson(const Chunk&                 aChunk,
                           BuildingBlockMask            aBuildingBlocks,
                           const ChunkMemoryLayoutInfo& aChunkMemLayout,
                           ReusableConversionBuffers*   aReusableConversionBuffers) {
    const auto chunkWidth  = ChunkMemoryLayoutInfoAccessor::getChunkWidth(aChunkMemLayout);
    const auto chunkHeight = ChunkMemoryLayoutInfoAccessor::getChunkHeight(aChunkMemLayout);

    json::Document doc;
    auto&          allocator = doc.GetAllocator();
    doc.SetObject();

    // Add CellKindId
    {
        json::Value array{json::kArrayType};
        if ((aBuildingBlocks & BuildingBlock::CELL_KIND_ID) != BuildingBlock::NONE) {
            array.Reserve(static_cast<unsigned>(chunkWidth * chunkHeight), allocator);
            cell::CellKindId cellKindId;
            for (hg::PZInteger y = 0; y < chunkHeight; y += 1) {
                for (hg::PZInteger x = 0; x < chunkWidth; x += 1) {
                    aChunk.getCellDataAtUnchecked(aChunkMemLayout, x, y, &cellKindId);
                    auto value = CellKindIdToJson(cellKindId, doc.GetAllocator());
                    array.PushBack(value, allocator);
                }
            }
        }
        doc.AddMember("cellKindIds", array, allocator);
    }

    // Add FloorSprite
    {
        json::Value array{json::kArrayType};
        if ((aBuildingBlocks & BuildingBlock::FLOOR_SPRITE) != BuildingBlock::NONE) {
            array.Reserve(static_cast<unsigned>(chunkWidth * chunkHeight), allocator);
            cell::FloorSprite floorSprite;
            for (hg::PZInteger y = 0; y < chunkHeight; y += 1) {
                for (hg::PZInteger x = 0; x < chunkWidth; x += 1) {
                    aChunk.getCellDataAtUnchecked(aChunkMemLayout, x, y, &floorSprite);
                    auto value = FloorSpriteToJson(floorSprite, doc.GetAllocator());
                    array.PushBack(value, allocator);
                }
            }
        }
        doc.AddMember("floorSprites", array, allocator);
    }

    // Add WallSprite
    {
        json::Value array{json::kArrayType};
        if ((aBuildingBlocks & BuildingBlock::WALL_SPRITE) != BuildingBlock::NONE) {
            array.Reserve(static_cast<unsigned>(chunkWidth * chunkHeight), allocator);
            cell::WallSprite wallSprite;
            for (hg::PZInteger y = 0; y < chunkHeight; y += 1) {
                for (hg::PZInteger x = 0; x < chunkWidth; x += 1) {
                    aChunk.getCellDataAtUnchecked(aChunkMemLayout, x, y, &wallSprite);
                    auto value = WallSpriteToJson(wallSprite, doc.GetAllocator());
                    array.PushBack(value, allocator);
                }
            }
        }
        doc.AddMember("wallSprites", array, allocator);
    }

    // Add SpatialInfo
    {
        json::Value array{json::kArrayType};
        if ((aBuildingBlocks & BuildingBlock::SPATIAL_INFO) != BuildingBlock::NONE) {
            array.Reserve(static_cast<unsigned>(chunkWidth * chunkHeight), allocator);
            cell::SpatialInfo spatialInfo;
            for (hg::PZInteger y = 0; y < chunkHeight; y += 1) {
                for (hg::PZInteger x = 0; x < chunkWidth; x += 1) {
                    aChunk.getCellDataAtUnchecked(aChunkMemLayout, x, y, &spatialInfo);
                    auto value = SpatialInfoToJson(spatialInfo, doc.GetAllocator());
                    array.PushBack(value, allocator);
                }
            }
        }
        doc.AddMember("spatialInfos", array, allocator);
    }

    // Add RendererAuxData
    {
        json::Value array{json::kArrayType};
        if ((aBuildingBlocks & BuildingBlock::RENDERER_AUX_DATA) != BuildingBlock::NONE) {
            array.Reserve(static_cast<unsigned>(chunkWidth * chunkHeight), allocator);
            cell::RendererAuxData rendererAuxData;
            for (hg::PZInteger y = 0; y < chunkHeight; y += 1) {
                for (hg::PZInteger x = 0; x < chunkWidth; x += 1) {
                    aChunk.getCellDataAtUnchecked(aChunkMemLayout, x, y, &rendererAuxData);
                    auto value = RendererAuxDataToJson(rendererAuxData, doc.GetAllocator());
                    array.PushBack(value, allocator);
                }
            }
        }
        doc.AddMember("rendererAuxData", array, allocator);
    }

    // Add UserData
    {
        json::Value array{json::kArrayType};
        if ((aBuildingBlocks & BuildingBlock::USER_DATA) != BuildingBlock::NONE) {
            array.Reserve(static_cast<unsigned>(chunkWidth * chunkHeight), allocator);
            cell::UserData userData;
            for (hg::PZInteger y = 0; y < chunkHeight; y += 1) {
                for (hg::PZInteger x = 0; x < chunkWidth; x += 1) {
                    aChunk.getCellDataAtUnchecked(aChunkMemLayout, x, y, &userData);
                    auto value = UserDataToJson(userData, doc.GetAllocator());
                    array.PushBack(value, allocator);
                }
            }
        }
        doc.AddMember("userData", array, allocator);
    }

    // Add extension
    const auto* extension = aChunk.getExtension();
    if (extension) {
        const auto  method = extension->getPreferredSerializationMethod();
        const auto& kind   = ExtensionKindToString(method);

        doc.AddMember("extension_kind", json::Value{kind, allocator}.Move(), allocator);

        if (method != ChunkExtensionInterface::SerializationMethod::NONE) {
            if (aReusableConversionBuffers != nullptr) {
                Base64Encode(*extension,
                             method,
                             aReusableConversionBuffers->stream,
                             aReusableConversionBuffers->string);

                doc.AddMember("extension_data",
                              json::Value{aReusableConversionBuffers->string, allocator},
                              allocator);
            } else {
                ReusableConversionBuffers defaultBuffers;
                Base64Encode(*extension, method, defaultBuffers.stream, defaultBuffers.string);

                doc.AddMember("extension_data",
                              json::Value{defaultBuffers.string, allocator},
                              allocator);
            }
        }
    } else {
        const auto& kind = ExtensionKindToString(std::nullopt);
        doc.AddMember("extension_kind", json::Value{kind, allocator}.Move(), allocator);
    }

    return doc;
}

Chunk JsonToChunk(const json::Document&        aJsonDocument,
                  const ChunkMemoryLayoutInfo& aChunkMemLayout,
                  const ChunkExtensionFactory& aChunkExtensionFactory,
                  ReusableConversionBuffers*   aReusableConversionBuffers) {
    Chunk chunk{aChunkMemLayout};

    const auto chunkWidth  = ChunkMemoryLayoutInfoAccessor::getChunkWidth(aChunkMemLayout);
    const auto chunkHeight = ChunkMemoryLayoutInfoAccessor::getChunkHeight(aChunkMemLayout);

    // Read CellKindId
    {
        ENSURE_JSON_CONTAINS(aJsonDocument, "cellKindIds", Array);
        const auto len = aJsonDocument["cellKindIds"].GetArray().Size();

        if (len > 0) {
            HG_HARD_ASSERT(len == static_cast<unsigned>(chunkWidth * chunkHeight));
            auto             iter = aJsonDocument["cellKindIds"].GetArray().begin();
            cell::CellKindId cellKindId;
            for (hg::PZInteger y = 0; y < chunkHeight; y += 1) {
                for (hg::PZInteger x = 0; x < chunkWidth; x += 1) {
                    cellKindId = JsonToCellKindId(*iter);
                    chunk.setCellDataAtUnchecked(aChunkMemLayout, x, y, &cellKindId);
                    ++iter;
                }
            }
        }
    }

    // Read FloorSprite
    {
        ENSURE_JSON_CONTAINS(aJsonDocument, "floorSprites", Array);
        const auto len = aJsonDocument["floorSprites"].GetArray().Size();

        if (len > 0) {
            HG_HARD_ASSERT(len == static_cast<unsigned>(chunkWidth * chunkHeight));
            auto              iter = aJsonDocument["floorSprites"].GetArray().begin();
            cell::FloorSprite floorSprite;
            for (hg::PZInteger y = 0; y < chunkHeight; y += 1) {
                for (hg::PZInteger x = 0; x < chunkWidth; x += 1) {
                    floorSprite = JsonToFloorSprite(*iter);
                    chunk.setCellDataAtUnchecked(aChunkMemLayout, x, y, &floorSprite);
                    ++iter;
                }
            }
        }
    }

    // Read WallSprite
    {
        ENSURE_JSON_CONTAINS(aJsonDocument, "wallSprites", Array);
        const auto len = aJsonDocument["wallSprites"].GetArray().Size();

        if (len > 0) {
            HG_HARD_ASSERT(len == static_cast<unsigned>(chunkWidth * chunkHeight));
            auto             iter = aJsonDocument["wallSprites"].GetArray().begin();
            cell::WallSprite wallSprite;
            for (hg::PZInteger y = 0; y < chunkHeight; y += 1) {
                for (hg::PZInteger x = 0; x < chunkWidth; x += 1) {
                    wallSprite = JsonToWallSprite(*iter);
                    chunk.setCellDataAtUnchecked(aChunkMemLayout, x, y, &wallSprite);
                    ++iter;
                }
            }
        }
    }

    // Read SpatialInfo
    {
        ENSURE_JSON_CONTAINS(aJsonDocument, "spatialInfos", Array);
        const auto len = aJsonDocument["spatialInfos"].GetArray().Size();

        if (len > 0) {
            HG_HARD_ASSERT(len == static_cast<unsigned>(chunkWidth * chunkHeight));
            auto              iter = aJsonDocument["spatialInfos"].GetArray().begin();
            cell::SpatialInfo spatialInfo;
            for (hg::PZInteger y = 0; y < chunkHeight; y += 1) {
                for (hg::PZInteger x = 0; x < chunkWidth; x += 1) {
                    spatialInfo = JsonToSpatialInfo(*iter);
                    chunk.setCellDataAtUnchecked(aChunkMemLayout, x, y, &spatialInfo);
                    ++iter;
                }
            }
        }
    }

    // Read RendererAuxData
    {
        ENSURE_JSON_CONTAINS(aJsonDocument, "rendererAuxData", Array);
        const auto len = aJsonDocument["rendererAuxData"].GetArray().Size();

        if (len > 0) {
            HG_HARD_ASSERT(len == static_cast<unsigned>(chunkWidth * chunkHeight));
            auto                  iter = aJsonDocument["rendererAuxData"].GetArray().begin();
            cell::RendererAuxData rendererAuxData;
            for (hg::PZInteger y = 0; y < chunkHeight; y += 1) {
                for (hg::PZInteger x = 0; x < chunkWidth; x += 1) {
                    rendererAuxData = JsonToRendererAuxData(*iter);
                    chunk.setCellDataAtUnchecked(aChunkMemLayout, x, y, &rendererAuxData);
                    ++iter;
                }
            }
        }
    }

    // Read UserData
    {
        ENSURE_JSON_CONTAINS(aJsonDocument, "userData", Array);
        const auto len = aJsonDocument["userData"].GetArray().Size();

        if (len > 0) {
            HG_HARD_ASSERT(len == static_cast<unsigned>(chunkWidth * chunkHeight));
            auto           iter = aJsonDocument["userData"].GetArray().begin();
            cell::UserData userData;
            for (hg::PZInteger y = 0; y < chunkHeight; y += 1) {
                for (hg::PZInteger x = 0; x < chunkWidth; x += 1) {
                    userData = JsonToUserData(*iter);
                    chunk.setCellDataAtUnchecked(aChunkMemLayout, x, y, &userData);
                    ++iter;
                }
            }
        }
    }

    // Read extension
    ENSURE_JSON_CONTAINS(aJsonDocument, "extension_kind", String);

    auto extension = aChunkExtensionFactory ? aChunkExtensionFactory(chunk) : nullptr;

    const auto kind = StringToExtensionKind(aJsonDocument["extension_kind"].GetString());
    if (kind.has_value()) {
        if (extension != nullptr) {
            ENSURE_JSON_CONTAINS(aJsonDocument, "extension_data", String);

            const auto& value               = aJsonDocument["extension_data"];
            const auto* encodedExtString    = value.GetString();
            const auto  encodedExtStringLen = value.GetStringLength();

            if (aReusableConversionBuffers != nullptr) {
                Base64Decode(*extension,
                             *kind,
                             encodedExtString,
                             encodedExtStringLen,
                             aReusableConversionBuffers->string);
            } else {
                ReusableConversionBuffers defaultBuffers;
                Base64Decode(*extension,
                             *kind,
                             encodedExtString,
                             encodedExtStringLen,
                             defaultBuffers.string);
            }
        } else {
            HG_LOG_WARN(
                LOG_ID,
                "Found serialized data for chunk extension but failed to instantiate the extension.");
        }
    }

    chunk.setExtension(std::move(extension));

    return chunk;
}

std::string ChunkToJsonString(const Chunk&                 aChunk,
                              BuildingBlockMask            aBuildingBlocks,
                              const ChunkMemoryLayoutInfo& aChunkMemLayout,
                              ReusableConversionBuffers*   aReusableConversionBuffers) {
#if HG_BUILD_TYPE == HG_DEBUG
#define JsonWriter json::PrettyWriter
#else
#define JsonWriter json::Writer
#endif

    const auto doc = ChunkToJson(aChunk, aBuildingBlocks, aChunkMemLayout, aReusableConversionBuffers);

    json::StringBuffer             stringbuf;
    JsonWriter<json::StringBuffer> writer(stringbuf);
    doc.Accept(writer);

    return stringbuf.GetString();

#undef JsonWriter
}

Chunk JsonStringToChunk(std::string                  aJsonString,
                        const ChunkMemoryLayoutInfo& aChunkMemLayout,
                        const ChunkExtensionFactory& aChunkExtensionFactory,
                        ReusableConversionBuffers*   aReusableConversionBuffers) {
    json::Document doc;
    doc.ParseInsitu(aJsonString.data());

    auto result = JsonToChunk(doc, aChunkMemLayout, aChunkExtensionFactory, aReusableConversionBuffers);

    return result;
}

} // namespace detail
} // namespace gridgoblin
} // namespace jbatnozic
