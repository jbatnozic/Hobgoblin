// Copyright 2024 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#include <GridGoblin/World/World_config.hpp>

#include <GridGoblin/Private/Model_conversions.hpp>

#include <Hobgoblin/Unicode.hpp>

#include <rapidjson/document.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/stringbuffer.h>

#include <fstream>

namespace jbatnozic {
namespace gridgoblin {

namespace {
namespace json = rapidjson;

json::Document ConfigToJson(const WorldConfig& aConfig) {
    json::Document document;
    auto&          allocator = document.GetAllocator();

    json::Value& value = document;
    value.SetObject();

    // clang-format off
    value.AddMember("chunkCountX",                 json::Value{aConfig.chunkCountX}.Move(), allocator);
    value.AddMember("chunkCountY",                 json::Value{aConfig.chunkCountY}.Move(), allocator);
    value.AddMember("cellsPerChunkX",              json::Value{aConfig.cellsPerChunkX}.Move(), allocator);
    value.AddMember("cellsPerChunkY",              json::Value{aConfig.cellsPerChunkY}.Move(), allocator);
    value.AddMember("cellResolution",              json::Value{aConfig.cellResolution}.Move(), allocator);
    value.AddMember("wallHeight",                  json::Value{aConfig.wallHeight}.Move(), allocator);
    value.AddMember("maxCellOpenness",             json::Value{aConfig.maxCellOpenness}.Move(), allocator);
    value.AddMember("maxLoadedNonessentialChunks", json::Value{aConfig.maxLoadedNonessentialChunks}.Move(), allocator);
    // clang-format on

    value.AddMember("chunkDirectoryPath",
                    json::Value{aConfig.chunkDirectoryPath.string(), allocator}.Move(),
                    allocator);

    return document;
}

#define ENSURE_JSON_CONTAINS(_value_, _member_name_, _member_type_)                                 \
    do {                                                                                            \
        if (!(_value_).HasMember(_member_name_) || !(_value_)[_member_name_].Is##_member_type_()) { \
            using namespace detail;                                                                 \
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

WorldConfig JsonToConfig(const json::Value& aJson) {
    return {.chunkCountX     = GetIntMember<hg::PZInteger>(aJson, "chunkCountX"),
            .chunkCountY     = GetIntMember<hg::PZInteger>(aJson, "chunkCountY"),
            .cellsPerChunkX  = GetIntMember<hg::PZInteger>(aJson, "cellsPerChunkX"),
            .cellsPerChunkY  = GetIntMember<hg::PZInteger>(aJson, "cellsPerChunkY"),
            .cellResolution  = GetFloatMember<float>(aJson, "cellResolution"),
            .wallHeight      = GetFloatMember<float>(aJson, "wallHeight"),
            .maxCellOpenness = GetIntMember<std::uint8_t>(aJson, "maxCellOpenness"),
            .maxLoadedNonessentialChunks =
                GetIntMember<hg::PZInteger>(aJson, "maxLoadedNonessentialChunks"),
            .chunkDirectoryPath = GetStringMember(aJson, "chunkDirectoryPath")};
}
} // namespace

void WorldConfig::saveToFile(const WorldConfig& aConfig, const std::filesystem::path& aFilePath) {
#if HG_BUILD_TYPE == HG_DEBUG
#define JsonWriter json::PrettyWriter
#else
#define JsonWriter json::Writer
#endif

    const auto doc = ConfigToJson(aConfig);

    json::StringBuffer             stringbuf;
    JsonWriter<json::StringBuffer> writer(stringbuf);
    doc.Accept(writer);

#undef JsonWriter

    std::ofstream file{aFilePath, std::ios::out | std::ios::trunc | std::ios::binary};
    file.write(stringbuf.GetString(), stringbuf.GetSize());
}

WorldConfig WorldConfig::loadFromFile(const std::filesystem::path& aFilePath) {
    auto unicodeContents = hg::LoadWholeFile(aFilePath);
    auto strContents     = hg::UniStrConv(hg::TO_UTF8_STD_STRING, unicodeContents);

    json::Document doc;
    doc.ParseInsitu(strContents.data());

    return JsonToConfig(doc);
}

} // namespace gridgoblin
} // namespace jbatnozic
