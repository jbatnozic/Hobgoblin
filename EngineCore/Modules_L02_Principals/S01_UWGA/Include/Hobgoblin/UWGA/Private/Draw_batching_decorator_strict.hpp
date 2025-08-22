// Copyright 2025 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#ifndef UHOBGOBLIN_UWGA_PRIVATE_DRAW_BATCHING_DECORATOR_STRICT_HPP
#define UHOBGOBLIN_UWGA_PRIVATE_DRAW_BATCHING_DECORATOR_STRICT_HPP

#include <Hobgoblin/UWGA/Canvas.hpp>

#include <type_traits>

#include <Hobgoblin/Private/Pmacro_define.hpp>

HOBGOBLIN_NAMESPACE_BEGIN
namespace uwga {
namespace detail {

//! TODO(add description)
// clang-format off
template <class taCanvasImpl,
          T_ENABLE_IF(std::is_base_of_v<Canvas, taCanvasImpl>)>
// clang-format on
class StructDrawBatchingDecorator : public taCanvasImpl {
public:
    using Super = taCanvasImpl;

    // Inherit all the constructors
    using Super::Super;

    // TODO: Actual implementation
};

} // namespace detail
} // namespace uwga
HOBGOBLIN_NAMESPACE_END

#include <Hobgoblin/Private/Pmacro_undef.hpp>
#include <Hobgoblin/Private/Short_namespace.hpp>

#endif // !UHOBGOBLIN_UWGA_PRIVATE_DRAW_BATCHING_DECORATOR_STRICT_HPP
