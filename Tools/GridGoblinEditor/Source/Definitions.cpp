// Copyright 2024 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#include "Definitions.hpp"

#include <rapidjson/document.h>

namespace jbatnozic {
namespace gridgoblin {
namespace editor {

namespace json = rapidjson;

namespace {
#define ENSURE_JSON_CONTAINS(_value_, _member_name_, _member_type_)                                 \
    do {                                                                                            \
        if (!(_value_).HasMember(_member_name_) || !(_value_)[_member_name_].Is##_member_type_()) { \
            HG_THROW_TRACED(JsonParseError, 0, "No member '{}' found.", _member_name_);             \
        }                                                                                           \
    } while (false)

template <class taIntegral>
taIntegral GetIntMember(const json::Value& aJsonValue, const char* aMemberName) {
    static_assert(std::is_integral<taIntegral>::value, "GetIntMember only works with integral types.");

    ENSURE_JSON_CONTAINS(aJsonValue, aMemberName, Int);

    return static_cast<taIntegral>(aJsonValue[aMemberName].GetInt());
}

template <class taFloatingPoint>
taFloatingPoint GetFloatMember(const json::Value& aJsonValue, const char* aMemberName) {
    if constexpr (std::is_same_v<taFloatingPoint, float>) {
        ENSURE_JSON_CONTAINS(aJsonValue, aMemberName, Float);
        return static_cast<float>(aJsonValue[aMemberName].GetFloat());
    } else if constexpr (std::is_same_v<taFloatingPoint, double>) {
        ENSURE_JSON_CONTAINS(aJsonValue, aMemberName, Double);
        return static_cast<double>(aJsonValue[aMemberName].GetDouble());
    } else {
        static_assert(false, "GetFloatMember only works with floating-point types.");
    }

    return static_cast<taFloatingPoint>(0);
}

std::string GetStringMember(const json::Value& aJsonValue, const char* aMemberName) {
    ENSURE_JSON_CONTAINS(aJsonValue, aMemberName, String);
    const char* chars  = aJsonValue[aMemberName].GetString();
    const auto  length = aJsonValue[aMemberName].GetStringLength();
    return std::string(chars, length);
}

CellDefinitions JsonToCellDefinitions(const json::Value& aJson) {
    CellDefinitions result;
    
    for (json::SizeType i = 0; i < aJson.Size(); i += 1) {
        auto& jsonCellDef = aJson[i];
        auto name = GetStringMember(jsonCellDef, "definitionName");

        ENSURE_JSON_CONTAINS(jsonCellDef, "wall", Object);
        ENSURE_JSON_CONTAINS(jsonCellDef, "floor", Object);

        std::optional<CellModel::Wall> wall;
        if (auto& jsonWall = jsonCellDef["wall"]; jsonWall.MemberCount() > 0) {
            wall.emplace();
            wall->spriteId = GetIntMember<int>(jsonWall, "spriteId");
            wall->spriteId_reduced = GetIntMember<int>(jsonWall, "spriteId_reduced");
            wall->shape = StringToShape(GetStringMember(jsonWall, "shape"));
        }

        std::optional<CellModel::Floor> floor;
        if (auto& jsonFloor = jsonCellDef["floor"]; jsonFloor.MemberCount() > 0) {
            floor.emplace();
            floor->spriteId = GetIntMember<int>(jsonFloor, "spriteId");
        }

        auto userData = GetIntMember<std::int64_t>(jsonCellDef, "userData");

        result[name] = CellDefinition{wall, floor, userData};
    }

    return result;
};

WorldDefinition JsonToWorldDefinition(const json::Value& aJson) {
    WorldDefinition result;

    result.chunkCountX    = GetIntMember<decltype(result.chunkCountX)>(aJson, "chunkCountX");
    result.chunkCountY    = GetIntMember<decltype(result.chunkCountY)>(aJson, "chunkCountY");
    result.cellsPerChunkX = GetIntMember<decltype(result.cellsPerChunkX)>(aJson, "cellsPerChunkX");
    result.cellsPerChunkY = GetIntMember<decltype(result.cellsPerChunkY)>(aJson, "cellsPerChunkY");
    result.cellResolution = GetFloatMember<decltype(result.cellResolution)>(aJson, "cellResolution");
    result.defaultCellDefinition = GetStringMember(aJson, "defaultCellDefinition");

    return result;
};
} // namespace

AllDefinitions JsonStringToAllDefinitions(std::string aJsonString) {
    AllDefinitions result;

    json::Document doc;
    doc.ParseInsitu(aJsonString.data());

    ENSURE_JSON_CONTAINS(doc, "cells", Array);
    ENSURE_JSON_CONTAINS(doc, "world", Object);

    result.cellDefinitions = JsonToCellDefinitions(doc["cells"]);
    result.worldDefinition = JsonToWorldDefinition(doc["world"]);

    return result;
}

} // namespace editor
} // namespace gridgoblin
} // namespace jbatnozic
