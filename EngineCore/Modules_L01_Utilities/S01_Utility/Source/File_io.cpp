// Copyright 2024 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#include <Hobgoblin/Utility/File_io.hpp>

#include <fstream>

#include <Hobgoblin/Private/Pmacro_define.hpp>

HOBGOBLIN_NAMESPACE_BEGIN
namespace util {

std::string SlurpFileBytes(const std::filesystem::path& aPath) {
    std::ifstream fileStream{aPath, std::ios::in | std::ios::binary};
    if (!fileStream.is_open()) {
        HG_THROW_TRACED(IOError,
                        0,
                        "Could not open file '{}'; file does not exist!",
                        aPath.filename().string());
    }

    std::string fileContents{std::istreambuf_iterator<char>{fileStream},
                             std::istreambuf_iterator<char>()};

    if (fileStream.fail()) {
        HG_THROW_TRACED(IOError,
                        0,
                        "Error encountered while reading file '{}'.",
                        aPath.filename().string());
    }

    return fileContents;
}

} // namespace util
HOBGOBLIN_NAMESPACE_END

#include <Hobgoblin/Private/Pmacro_undef.hpp>
