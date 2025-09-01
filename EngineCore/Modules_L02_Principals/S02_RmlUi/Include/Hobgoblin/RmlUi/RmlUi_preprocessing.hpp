// Copyright 2024 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#ifndef UHOBGOBLIN_RMLUI_RMLUI_PREPROCESSING_HPP
#define UHOBGOBLIN_RMLUI_RMLUI_PREPROCESSING_HPP

#include <Hobgoblin/HGExcept.hpp>
#include <Hobgoblin/UWGA/System.hpp>

#include <filesystem>

#include <Hobgoblin/Private/Pmacro_define.hpp>
HOBGOBLIN_NAMESPACE_BEGIN
namespace rml {

//! Thrown when preprocessing an .rcss file fails.
class PreprocessingError : TracedRuntimeError {
public:
    using TracedRuntimeError::TracedRuntimeError;
};

//! TODO(add description)
void PreprocessRcssFile(std::filesystem::path aFilePath, const uwga::System& aGraphicsSystem);

} // namespace rml
HOBGOBLIN_NAMESPACE_END
#include <Hobgoblin/Private/Pmacro_undef.hpp>
#include <Hobgoblin/Private/Short_namespace.hpp>

#endif // !UHOBGOBLIN_RMLUI_RMLUI_PREPROCESSING_HPP
