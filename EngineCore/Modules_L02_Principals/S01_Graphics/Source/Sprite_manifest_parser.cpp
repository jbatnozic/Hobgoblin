// Copyright 2024 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#include "Sprite_manifest_parser.hpp"

#include <Hobgoblin/HGExcept.hpp>
#include <Hobgoblin/Logging.hpp>
#include <Hobgoblin/Unicode.hpp>

#include <cstdint>
#include <string>
#include <unordered_map>
#include <variant>

#include <Hobgoblin/Private/Pmacro_define.hpp>

HOBGOBLIN_NAMESPACE_BEGIN
namespace gr {

namespace {
constexpr auto LOG_ID = "Hobgoblin.Graphics";

constexpr char16_t CR          = 0x0D;
constexpr char16_t LF          = 0x0A;
constexpr char16_t COMMA       = ',';
constexpr char16_t MONKEY      = '@';
constexpr char16_t DQUOTE      = '"';  //!< Double quote
constexpr char16_t BSLASH      = '\\'; //!< Backslash
constexpr char16_t PAREN_OPEN  = '(';
constexpr char16_t PAREN_CLOSE = ')';

const auto TAG_ENUMERATION = HG_UNISTR("@enumeration");
const auto TAG_TEXTURE     = HG_UNISTR("@texture");

const auto FUNC_ADD_SPRITE    = HG_UNISTR("addSprite");
const auto FUNC_ADD_SUBSPRITE = HG_UNISTR("addSubsprite");

const auto WHITESPACE_REGEX_PATTERN  = HG_UNISTR(R"_(\s*)_");
const auto STRIPPING_REGEX_PATTERN   = HG_UNISTR(R"_(^\s*(.*?)\s*(\/\/.*)?$)_");
const auto ENUMERATION_REGEX_PATTERN = HG_UNISTR(R"_(\s*([_a-zA-Z0-9]+)\s*=\s*([0-9xABCDEF]+)\s*)_");
const auto ENUMERATION_FUNC_REGEX_PATTERN =
    HG_UNISTR(R"_(\s*([_a-zA-Z0-9]+)\s*=\s*([_a-zA-Z0-9]+)\(\s*([0-9xABCDEF]+)\s*\)\s*)_");
const auto TEXTURE_REGEX_PATTERN = HG_UNISTR(
    R"_(\s*@texture\(\s*([0-9xABCDEF]+)\s*,\s*([0-9xABCDEF]+)\s*,\s*([a-zA-Z]+)\s*\)\s*:\s*)_");

TexturePackingHeuristic StringToTexturePackingHeuristic(const std::string_view& aString) {
    using namespace std::literals;
    if (aString == "BestShortSideFit"sv) {
        return TexturePackingHeuristic::BestShortSideFit;
    } else if (aString == "BestLongSideFit"sv) {
        return TexturePackingHeuristic::BestLongSideFit;
    } else if (aString == "BestAreaFit"sv) {
        return TexturePackingHeuristic::BestAreaFit;
    } else if (aString == "BottomLeftRule"sv) {
        return TexturePackingHeuristic::BottomLeftRule;
    } else if (aString == "ContactPointRule"sv) {
        return TexturePackingHeuristic::ContactPointRule;
    }

    HG_THROW_TRACED(SpriteManifestProcessingError,
                    0,
                    "Unknown TexturePackingHeuristic: >>{}<<.",
                    aString);
}

class ManifestParser {
public:
    ManifestParser(const std::filesystem::path& aManifestFilePath)
        : _manifestDirectory{aManifestFilePath.parent_path()}
        , _ustring{LoadWholeFile(aManifestFilePath)}
        , _whitespaceRegex{WHITESPACE_REGEX_PATTERN}
        , _strippingRegex{STRIPPING_REGEX_PATTERN}
        , _enumerationRegex{ENUMERATION_REGEX_PATTERN}
        , _enumerationFuncRegex{ENUMERATION_FUNC_REGEX_PATTERN}
        , _textureRegex{TEXTURE_REGEX_PATTERN} {}

    void parse(SpriteLoader& aSpriteLoader) {
        _stripInput();

        for (;;) {
            _findTag();
            if (_hand >= _ustring.length()) {
                break;
            }

            if (_ustring.tempSubString(_hand, TAG_ENUMERATION.length()) == TAG_ENUMERATION) {
                _readEnumeration();
            } else if (_ustring.tempSubString(_hand, TAG_TEXTURE.length()) == TAG_TEXTURE) {
                _readTexture(aSpriteLoader);
            } else {
                HG_THROW_TRACED(SpriteManifestProcessingError,
                                0,
                                "Unknown tag encountered at >>{}<<.",
                                UniStrConv(TO_ASCII_STD_STRING, _ustring.tempSubString(_hand, 32)));
            }
        }
    }

private:
    std::filesystem::path _manifestDirectory;

    UnicodeString _ustring;
    std::int32_t  _hand;

    URegex _whitespaceRegex;
    URegex _strippingRegex;
    URegex _enumerationRegex;
    URegex _enumerationFuncRegex;
    URegex _textureRegex;

    std::unordered_map<std::string, PZInteger> _enumerationMappings;

    // MARK: Stripping

    //! For each line in the input: remove comments, leading and trailing whitespace, then newline chars
    void _stripInput() {
        _hand = 0;

        UnicodeString workStr;

        const auto length = _ustring.length();
        while (_hand < length) {
            for (std::int32_t i = _hand; i < length; i += 1) {
                if ((i + 1 < length) && _ustring[i] == CR && _ustring[i + 1] == LF) {
                    _strip_processLine(_hand, i - 1, workStr);
                    _hand = i + 2;
                    break;
                } else if (_ustring[i] == LF) {
                    _strip_processLine(_hand, i - 1, workStr);
                    _hand = i + 1;
                    break;
                } else if (i == length - 1) {
                    _strip_processLine(_hand, i, workStr);
                    _hand = i + 1;
                    break;
                } else {
                    continue;
                }
            }
        }

        std::swap(workStr, _ustring);
        _hand = 0;

        HG_LOG_DEBUG(LOG_ID,
                     "Input string after stripping: >>{}<<",
                     UniStrConv(TO_ASCII_STD_STRING, _ustring));
    }

    void _strip_processLine(std::int32_t   aUStringStartInclusive,
                            std::int32_t   aUStringEndInclusive,
                            UnicodeString& aAppendTo) {
        const auto fragment =
            _ustring.tempSubStringBetween(aUStringStartInclusive, aUStringEndInclusive + 1);

        UMatchResults results;
        if (RegexMatch(fragment, _strippingRegex, results)) {
            HG_HARD_ASSERT(results.getGroupCount() >= 2);
            aAppendTo.append(results[1]);
        } else {
            HG_UNREACHABLE("Fragment did not match to stripping regex: >>{}<<",
                           UniStrConv(TO_ASCII_STD_STRING, fragment));
        }
    }

    // MARK: Search

    void _findTag() {
        const auto length = _ustring.length();
        while (_hand < length && _ustring[_hand] != MONKEY) {
            if (!std::isspace(_ustring[_hand])) {
                HG_THROW_TRACED(SpriteManifestProcessingError,
                                0,
                                "Expected tag starting with @, got >>{}<< ({}).",
                                (char)_ustring[_hand],
                                (int)_ustring[_hand]);
            }
            ++_hand;
        }
    }

    // MARK: Enumeration

    void _readEnumeration() {
        _hand += TAG_ENUMERATION.length() + 1; // +1 for colon

        const auto length = _ustring.length();
        while (_hand < length) {
            for (std::int32_t i = _hand; i < length; i += 1) {
                if (_ustring[i] == COMMA) {
                    _processEnumeration(_hand, i - 1);
                    _hand = i + 1;
                    break;
                } else if (_ustring[i] == MONKEY) {
                    _processEnumeration(_hand, i - 1);
                    _hand = i;
                    goto FUNCTION_END;
                } else if (i == length - 1) {
                    _processEnumeration(_hand, i);
                    _hand = i + 1;
                    goto FUNCTION_END;
                } else {
                    continue;
                }
            }
        }

    FUNCTION_END:
        NO_OP();
    }

    void _processEnumeration(std::int32_t aUStringStartInclusive, std::int32_t aUStringEndInclusive) {
        const auto fragment =
            _ustring.tempSubStringBetween(aUStringStartInclusive, aUStringEndInclusive + 1);
        HG_LOG_DEBUG(LOG_ID,
                     "_processEnumeration - fragment = >>{}<<",
                     UniStrConv(TO_ASCII_STD_STRING, fragment));

        UMatchResults results;
        if (RegexMatch(fragment, _enumerationRegex, results)) {
            HG_HARD_ASSERT(results.getGroupCount() >= 2);
            const auto identifier = UniStrConv(TO_ASCII_STD_STRING, results[1]);
            const auto value      = std::stoi(UniStrConv(TO_ASCII_STD_STRING, results[2]), nullptr, 0);
            _addEnumerationMapping(identifier, value, nullptr);
        } else if (RegexMatch(fragment, _enumerationFuncRegex, results)) {
            HG_HARD_ASSERT(results.getGroupCount() >= 3);
            const auto identifier = UniStrConv(TO_ASCII_STD_STRING, results[1]);
            const auto function   = results[2];
            const auto value      = std::stoi(UniStrConv(TO_ASCII_STD_STRING, results[3]), nullptr, 0);
            _addEnumerationMapping(identifier, value, &function);
        } else if (RegexMatch(fragment, _whitespaceRegex, results)) {
            return;
        } else {
            HG_THROW_TRACED(SpriteManifestProcessingError,
                            0,
                            "Could not match enumeration fragment >>{}<< to any regex.",
                            UniStrConv(TO_ASCII_STD_STRING, fragment));
        }
    }

    void _addEnumerationMapping(const std::string&   aIdentifier,
                                PZInteger            aValue,
                                const UnicodeString* aFunction) {
        if (_enumerationMappings.contains(aIdentifier)) {
            HG_THROW_TRACED(SpriteManifestProcessingError,
                            0,
                            "Identifier '{}' defined multiple times.",
                            aIdentifier);
        }

        if (aFunction) {
            HG_LOG_DEBUG(LOG_ID,
                         "_addEnumerationMapping: '{}' => {} (func: {})",
                         aIdentifier,
                         aValue,
                         UniStrConv(TO_ASCII_STD_STRING, *aFunction));
            HG_NOT_IMPLEMENTED("TODO");
        } else {
            HG_LOG_DEBUG(LOG_ID, "_addEnumerationMapping: '{}' => {}", aIdentifier, aValue);
            _enumerationMappings[aIdentifier] = aValue;
        }
    }

    // MARK: Texture

    void _readTexture(SpriteLoader& aSpriteLoader) {
        const auto fragment = _ustring.tempSubString(_hand, INT32_MAX);
        HG_LOG_DEBUG(LOG_ID,
                     "_readTexture - fragment = >>{}<<",
                     UniStrConv(TO_ASCII_STD_STRING, fragment));

        UMatchResults results;
        if (RegexMatchPartial(fragment, _textureRegex, results)) {
            HG_HARD_ASSERT(results.getGroupCount() >= 3);
            const auto texW = std::stoi(UniStrConv(TO_ASCII_STD_STRING, results[1]), nullptr, 0);
            const auto texH = std::stoi(UniStrConv(TO_ASCII_STD_STRING, results[2]), nullptr, 0);
            const auto packingHeuristicStr = UniStrConv(TO_ASCII_STD_STRING, results[3]);
            HG_LOG_DEBUG(LOG_ID,
                         "Found texture tag (w: {}, h: {}, heuristic: {}).",
                         texW,
                         texH,
                         packingHeuristicStr);
            const auto packingHeuristic = StringToTexturePackingHeuristic(packingHeuristicStr);

            _hand += results[0].length();
            auto textureBuilder = aSpriteLoader.startTexture(texW, texH);
            _processTextureSprites(*textureBuilder);
            float occupancy = 0.0f;
            textureBuilder->finalize(packingHeuristic, &occupancy);
            HG_LOG_INFO(LOG_ID,
                        "Texture of size ({}x{}) successfully created with packing heuristic '{}' and "
                        "{}% occupancy.",
                        texW,
                        texH,
                        packingHeuristicStr,
                        occupancy * 100.f);
        } else {
            HG_THROW_TRACED(SpriteManifestProcessingError,
                            0,
                            "@texture tag arguments not in expected format.");
        }
    }

    void _processTextureSprites(SpriteLoader::TextureBuilder& aTextureBuilder) {
        enum State {
            LOOK_FOR_START,
            LOOK_FOR_OPENING_PARENTHESIS,
            GATHER_ARGUMENTS,
            LOOK_FOR_END,
        };

        auto state = LOOK_FOR_START;

        bool isInQuotes = false;

        UnicodeString              current;
        std::vector<UnicodeString> words;

        const auto   length = _ustring.length();
        std::int32_t i;
        for (i = _hand; i < length; i += 1) {
            const auto c = _ustring[i];
            if (!isInQuotes) {
                switch (state) {
                case LOOK_FOR_START:
                    if (std::isspace(c)) {
                        continue;
                    } else {
                        current.append(c);
                        state = LOOK_FOR_OPENING_PARENTHESIS;
                    }
                    break;

                case LOOK_FOR_OPENING_PARENTHESIS:
                    if (c == PAREN_OPEN) {
                        words.emplace_back(std::move(current));
                        current.remove();
                        state = GATHER_ARGUMENTS;
                    } else {
                        current.append(c);
                    }
                    break;

                case GATHER_ARGUMENTS:
                    if (std::isspace(c)) {
                        // Do nothing
                    } else if (c == COMMA || c == PAREN_CLOSE) {
                        words.emplace_back(std::move(current));
                        current.remove();
                        if (c == PAREN_CLOSE) {
                            _texture_processItem(aTextureBuilder, words);
                            words.clear();
                            state = LOOK_FOR_END;
                        }
                    } else if (c == DQUOTE) {
                        current.append(c);
                        isInQuotes = true;
                    } else {
                        current.append(c);
                    }
                    break;

                case LOOK_FOR_END:
                    if (c == COMMA || std::isspace(c)) {
                        // Do nothing
                    } else if (c == MONKEY) {
                        goto END_FOR;
                    } else {
                        current.append(c);
                        state = LOOK_FOR_OPENING_PARENTHESIS;
                    }
                    break;

                default:
                    HG_UNREACHABLE("Invalid state value ({}).", (int)state);
                } // end_switch
            } else {
                if (c == DQUOTE && _ustring[i - 1] != BSLASH) {
                    current.append(c);
                    isInQuotes = false;
                } else {
                    current.append(c);
                }
            } // end_if
        } // end_for

    END_FOR:
        _hand = i;

        if (i >= length - 1) {
            if (state != LOOK_FOR_END) {
                HG_THROW_TRACED(SpriteManifestProcessingError,
                                0,
                                "Unexpected end while parsing texture (state {}).",
                                (int)state);
            }
        }
    }

    void _texture_processItem(SpriteLoader::TextureBuilder&     aTextureBuilder,
                              const std::vector<UnicodeString>& aWords) {
        HG_HARD_ASSERT(aWords.size() >= 1);

        /*
            addSprite(<sprite-id>, <path>) // sprite-id can be textual or numerical
            addSubsprite(<sprite-id>, <subsprite-index>, <path>) // sprite-id can be textual or
                                                                 // numerical, index is optional
        */

        const auto& func = aWords[0];

        if (func == FUNC_ADD_SPRITE) {
            if (aWords.size() != 3) {
                HG_THROW_TRACED(SpriteManifestProcessingError,
                                0,
                                "addSprite function expects 2 arguments, got {}.",
                                aWords.size() - 1);
            }
            const auto spriteId = _getSpriteId(aWords[1]);
            const auto path     = _getSpritePath(aWords[2]);
            if (std::holds_alternative<std::string>(spriteId)) {
                aTextureBuilder.addSprite(std::get<std::string>(spriteId), path);
            } else {
                aTextureBuilder.addSprite(std::get<PZInteger>(spriteId), path);
            }
        } else if (func == FUNC_ADD_SUBSPRITE) {
            if (aWords.size() != 3 && aWords.size() != 4) {
                HG_THROW_TRACED(SpriteManifestProcessingError,
                                0,
                                "addSprite function expects 2 or 3 arguments, got {}.",
                                aWords.size() - 1);
            }
            const auto spriteId = _getSpriteId(aWords[1]);
            const auto path     = _getSpritePath(aWords.back());
            if (std::holds_alternative<std::string>(spriteId)) {
                if (aWords.size() == 4) {
                    const auto subspriteIndex =
                        std::stoi(UniStrConv(TO_ASCII_STD_STRING, aWords[2]), nullptr, 0);
                    aTextureBuilder.addSubsprite(std::get<std::string>(spriteId), subspriteIndex, path);
                } else {
                    aTextureBuilder.addSubsprite(std::get<std::string>(spriteId), path);
                }
            } else {
                if (aWords.size() == 4) {
                    const auto subspriteIndex =
                        std::stoi(UniStrConv(TO_ASCII_STD_STRING, aWords[2]), nullptr, 0);
                    aTextureBuilder.addSubsprite(std::get<PZInteger>(spriteId), subspriteIndex, path);
                } else {
                    aTextureBuilder.addSubsprite(std::get<PZInteger>(spriteId), path);
                }
            }
        } else {
            HG_THROW_TRACED(SpriteManifestProcessingError,
                            0,
                            "Expected sprite or subsprite adding function but got >>{}<<.",
                            UniStrConv(TO_ASCII_STD_STRING, func));
        }
    }

    std::variant<std::string, PZInteger> _getSpriteId(const UnicodeString& aString) {
        HG_HARD_ASSERT(!aString.isEmpty());

        if (aString[0] == DQUOTE) {
            HG_HARD_ASSERT(aString.length() > 1);
            return {UniStrConv(TO_ASCII_STD_STRING, aString.tempSubString(1, aString.length() - 2))};
        } else if (std::isdigit(aString[0])) {
            const auto id = std::stoi(UniStrConv(TO_ASCII_STD_STRING, aString), nullptr, 0);
            return {id};
        } else {
            const auto key  = UniStrConv(TO_ASCII_STD_STRING, aString);
            const auto iter = _enumerationMappings.find(key);
            if (iter == _enumerationMappings.end()) {
                HG_THROW_TRACED(SpriteManifestProcessingError,
                                0,
                                "Undefined sprite enumeration '{}'.",
                                key);
            }
            return {iter->second};
        }
    }

    std::filesystem::path _getSpritePath(const UnicodeString& aString) {
        HG_HARD_ASSERT(aString.length() > 1);
        if (aString[0] != DQUOTE || aString[aString.length() - 1] != DQUOTE) {
            HG_THROW_TRACED(SpriteManifestProcessingError,
                            0,
                            "Path to sprite or subsprite must be a string and begin and end with "
                            "quotes. Instead got >>{}<<.",
                            UniStrConv(TO_ASCII_STD_STRING, aString));
        }

        const auto path = UniStrConv(TO_STD_PATH, aString.tempSubString(1, aString.length() - 2));

        if (path.is_absolute()) {
            return path;
        } else {
            return _manifestDirectory / path;
        }
    }
};
} // namespace

void ParseSpriteManifestFile(const std::filesystem::path& aPath, SpriteLoader& aSpriteLoader) {
    try {
        ManifestParser parser{aPath};
        parser.parse(aSpriteLoader);
    } catch (const SpriteManifestProcessingError& aEx) {
        throw;
    } catch (const TracedException& aEx) {
        throw aEx.cast<SpriteManifestProcessingError>();
    } catch (const std::exception& aEx) {
        HG_THROW_TRACED(
            SpriteManifestProcessingError,
            1,
            "Standard exception thrown while trying to process a sprite manifest file. Details: {}",
            aEx.what());
    } catch (...) {
        HG_THROW_TRACED(SpriteManifestProcessingError,
                        1,
                        "Non-standard exception thrown while trying to process a sprite manifest file. "
                        "Details not available.");
    }
}

} // namespace gr
HOBGOBLIN_NAMESPACE_END

#include <Hobgoblin/Private/Pmacro_undef.hpp>
