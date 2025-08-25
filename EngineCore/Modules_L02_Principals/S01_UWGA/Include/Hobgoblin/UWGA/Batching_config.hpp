// Copyright 2025 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#ifndef UHOBGOBLIN_UWGA_BATCHING_CONFIG_HPP
#define UHOBGOBLIN_UWGA_BATCHING_CONFIG_HPP

#include <Hobgoblin/Math/Vector.hpp>

#include <cmath>

#include <Hobgoblin/Private/Pmacro_define.hpp>

HOBGOBLIN_NAMESPACE_BEGIN
namespace uwga {

//! \brief defines how a `Canvas` object should handle draw batching.
//!
//! Draw batching combines multiple `draw()` calls made to a `Canvas` into a single call
//! to the underlying graphics API. This is important because context switches and GPU
//! command submissions are expensive operations. Even simple commands can take significant
//! time if sent individually in large numbers.
//!
//! However, not all draw calls are compatible with each other:
//! - sets of vertices using a primitive type other than POINTS, LINES, or TRIANGLES are not batchable.
//! - `draw()` calls that use different textures, transforms, blend modes and shaders are not batchable.
//!
//! With those limitations, batching is mostly useful for drawing large amounts of sprites.
//! `uwga::Sprite` objects internally use TRIANGLES as their primitive, and you'll usually be drawing
//! multiple sprites from the same sprite sheet (texture) and with the same render states (transform,
//! blend mode, shader) in sequence. However, it's best to purposefully try to arrange your sprites in
//! such a way that maximises potential batches, if at all possible.
struct BatchingConfig {
    //! We stated that sets of vertices (assuming a batchable primitive type) with the same
    //! render states - texture, transform, blend mode, shader - are batchable. However, the
    //! difficulty lies in defining and detecting what "same" means here. For transforms and
    //! blend modes, it's easy, because they're basically value types.
    //! On the other hand, we can easily imagine an example where the same underlying OpenGL
    //! texture if used for two subsequent `draw()` calls, but the texture itself is updated
    //! between the two calls. By identity, it's the same texture, but for purposes of batching,
    //! it's different. At least textures are mostly rarely updated, but for shaders this is
    //! more of a problem, because their uniforms can change often.
    //! Hence, there are several strategies as to how to handle this:
    //! - DISABLED - disable batching completely. This is the slowest of all strategies,
    //!              but always correct.
    //! - STRICT - track changes to textures and shaders in something like a versioning scheme.
    //!            this is a little more complicated, but should provide the correct results
    //!            in almost all use cases.
    //! - FAST_N_LOOSE - compare textures and shaders by identities (addresses, native handles, etc.).
    //!                  This covers *a lot* of cases (especially simple ones) correctly, but for the
    //!                  remaining ones, it relies on the programmer to call `Canvas::flush()` at the
    //!                  appropriate times - after changing the underlying data of a texture or shader
    //!                  that's being used.
    enum class Strategy {
        DISABLED,
        STRICT, //! \warning NOT YET IMPLEMENTED
        FAST_N_LOOSE
    };

    Strategy strategy;
};

} // namespace uwga
HOBGOBLIN_NAMESPACE_END

#include <Hobgoblin/Private/Pmacro_undef.hpp>
#include <Hobgoblin/Private/Short_namespace.hpp>

#endif // !UHOBGOBLIN_UWGA_BATCHING_CONFIG_HPP
