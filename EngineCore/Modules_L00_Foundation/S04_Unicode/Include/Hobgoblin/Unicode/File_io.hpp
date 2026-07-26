// Copyright 2024 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#ifndef UHOBGOBLIN_UNICODE_FILE_IO_HPP
#define UHOBGOBLIN_UNICODE_FILE_IO_HPP

#include <Hobgoblin/Unicode/Charset_detection.hpp>
#include <Hobgoblin/Unicode/Unicode_string.hpp>

#include <filesystem>
#include <optional>

#include <Hobgoblin/Private/Pmacro_define.hpp>

HOBGOBLIN_NAMESPACE_BEGIN

//! Loads the whole contents of a textual file into a unicode string.
//!
//! \param aPath Path to the file.
//! \param aCharset Encoding charset of the file. If none, the
//!                 function will try to detect it automatically.
//!
//! \throws IOError if file could not be opened or there was an error during reading.
//! \throws TracedRuntimeError if the file's charset could not be determined.
UnicodeString LoadWholeFile(const std::filesystem::path& aPath, std::optional<Charset> aCharset = {});

HOBGOBLIN_NAMESPACE_END

#include <Hobgoblin/Private/Pmacro_undef.hpp>
#include <Hobgoblin/Private/Short_namespace.hpp>

#endif // !UHOBGOBLIN_UNICODE_FILE_IO_HPP
