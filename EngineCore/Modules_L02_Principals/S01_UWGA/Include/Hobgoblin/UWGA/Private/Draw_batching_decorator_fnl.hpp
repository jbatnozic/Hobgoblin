// Copyright 2025 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#ifndef UHOBGOBLIN_UWGA_PRIVATE_DRAW_BATCHING_DECORATOR_FNL_HPP
#define UHOBGOBLIN_UWGA_PRIVATE_DRAW_BATCHING_DECORATOR_FNL_HPP

#include <Hobgoblin/Common/Branches.hpp>
#include <Hobgoblin/HGExcept.hpp>
#include <Hobgoblin/Math/Vector.hpp>
#include <Hobgoblin/UWGA/Anchor_utils.hpp>
#include <Hobgoblin/UWGA/Canvas.hpp>
#include <Hobgoblin/UWGA/Private/Draw_batching_utils.hpp>
#include <Hobgoblin/UWGA/Texture.hpp>
#include <Hobgoblin/UWGA/Vertex_array.hpp>

#include <cassert>
#include <memory>
#include <type_traits>

#include <Hobgoblin/Private/Pmacro_define.hpp>

HOBGOBLIN_NAMESPACE_BEGIN
namespace uwga {
namespace detail {

//! Draw batching decorator implementing the `FAST_N_LOOSE` batching strategy.
// clang-format off
template <class taCanvasImpl,
          T_ENABLE_IF(std::is_base_of_v<Canvas, taCanvasImpl>)>
// clang-format on
class FastNLooseDrawBatchingDecorator : public taCanvasImpl {
public:
    using Super               = taCanvasImpl;
    using PerformanceCounters = Super::PerformanceCounters;

    // Inherit all the constructors
    using Super::Super;

    void clear(Color aColor = COLOR_BLACK) override {
        _clear();
        Super::clear(aColor);
    }

    void draw(const Vertex*       aVertices,
              PZInteger           aVertexCount,
              PrimitiveType       aPrimitiveType,
              math::Vector2d      aAnchor,
              const RenderStates& aStates = RENDER_STATES_DEFAULT) override {
        ++_perfCnt.vcallCount;

        if (HG_UNLIKELY_CONDITION(aVertices == nullptr || aVertexCount == 0)) {
            HG_UNLIKELY_BRANCH;
            return;
        }

        if (_va.vertices.empty()) {
            if (!IsBatchable(aPrimitiveType)) {
                // This is the easiest case: the VA is empty and the new set of vertices is
                // non-batchable, so we just pass it onto the superclass.
                _superDraw(aVertices, aVertexCount, aPrimitiveType, aAnchor, aStates);
            } else {
            START_NEW_BATCH:
                // In this case, the VA is empty but the new set of vertices is batchable,
                // so we can start a new batch.
                _va.anchor        = aAnchor;
                _va.primitiveType = aPrimitiveType;
                _va.vertices.insert(_va.vertices.end(), aVertices, aVertices + aVertexCount);

                _perfCnt.currentAggregation = 1;

                _saveRenderStates(aStates);
            }
        } else {
            if (!IsBatchable(aPrimitiveType)) {
                // Another easy case: the VA is not empty but the new set of vertices is not
                // batchable, so we flush and then pass the vertices to the superclass.
                _flush();
                _superDraw(aVertices, aVertexCount, aPrimitiveType, aAnchor, aStates);
            } else {
                // The hardest case: with two nonempty but batchable sets of vertices, we have to
                // detect if they are compatible or not, and also deal with different anchors.
                // clang-format off
                const bool batchesCompatible =
                    (aPrimitiveType == _va.primitiveType) &&
                    (aStates.blendMode == _states.blendMode) &&
                    _isTextureCompatibleWithCurrentBranch(aStates.texture) &&
                    _isShaderCompatibleWithCurrentBranch(aStates.shader) &&
                    _isTransformCompatibleWithCurrentBatch(aStates.transform);
                // clang-format on

                if (!batchesCompatible) {
                    _flush();
                    goto START_NEW_BATCH;
                }

                const bool majorityIsNew = (pztos(aVertexCount) > _va.vertices.size());
                auto it = _va.vertices.insert(_va.vertices.end(), aVertices, aVertices + aVertexCount);
                ++_perfCnt.currentAggregation;
                if (!AreAnchorsApproxEq(aAnchor, _va.anchor)) {
                    const auto delta = math::VectorCast<float>(aAnchor - _va.anchor);
                    if (HG_LIKELY_CONDITION(!majorityIsNew)) {
                        HG_LIKELY_BRANCH;
                        auto end = _va.vertices.end();
                        for (; it != end; ++it) {
                            it->position += delta;
                        }
                    } else {
                        auto end = _va.vertices.begin();
                        std::swap(it, end);
                        for (; it != end; ++it) {
                            it->position -= delta;
                        }
                        _va.anchor = aAnchor;
                    }
                }
            }
        }
    }

    void flush() override {
        _flush();
        Super::flush();
    }

    const PerformanceCounters& getPerformanceCounters() const override {
        if (_perfCnt.maxAggregation < _perfCnt.currentAggregation) {
            _perfCnt.maxAggregation = _perfCnt.currentAggregation;
        }
        return _perfCnt;
    }

    void resetPerformanceCounters() override {
        _perfCnt = {};
    }

    PerformanceCounters getAndResetPerformanceCounters() override {
        PerformanceCounters result = _perfCnt;
        _perfCnt                   = {};
        return result;
    }

private:
    VertexArray _va;

    RenderStates _states;

    // NOTE: If a `RenderStates` object non-null Transform is ever passed to the batcher, it will
    //       clone it. On subsequent uses, it will reuse this object (just copy the transformation
    //       matrices into it). When a transform is s not needed, `_states.transform` will be set
    //       to null, but otherwise it will point to this object.
    std::unique_ptr<Transform> _auxTransform;

    struct PerformanceCountersExt : PerformanceCounters {
        PZInteger currentAggregation = 0;
    };

    mutable PerformanceCountersExt _perfCnt;

    void _clear() {
        _va.vertices.clear();
        _perfCnt.currentAggregation = 0;
    }

    void _superDraw(const Vertex*       aVertices,
                    PZInteger           aVertexCount,
                    PrimitiveType       aPrimitiveType,
                    math::Vector2d      aAnchor,
                    const RenderStates& aStates) {
        Super::draw(aVertices, aVertexCount, aPrimitiveType, aAnchor, aStates);
        ++_perfCnt.ucallCount;
    }

    void _flush() {
        _superDraw(_va.vertices.data(),
                   stopz(_va.vertices.size()),
                   _va.primitiveType,
                   _va.anchor,
                   _states);

        if (_perfCnt.maxAggregation < _perfCnt.currentAggregation) {
            _perfCnt.maxAggregation = _perfCnt.currentAggregation;
        }

        _clear();
    }

    void _saveRenderStates(const RenderStates& aStates) {
        _states.texture   = aStates.texture;
        _states.shader    = aStates.shader;
        _states.blendMode = aStates.blendMode;

        if (!aStates.transform) {
            _states.transform = nullptr;
        } else {
            if (!_auxTransform) {
                _auxTransform = aStates.transform->clone();
            } else {
                _auxTransform->setToCopyOf(*aStates.transform);
            }
            _states.transform = _auxTransform.get();
        }
    }

    bool _isTextureCompatibleWithCurrentBranch(const Texture* aTexture) {
        const auto selector = static_cast<int>(!!_states.texture) | (static_cast<int>(!!aTexture) << 1);

        switch (selector) {
        case 0: // both are null
            return true;

        case 1: // aTexture is null
        case 2: // _states.texture is null
            return false;

        case 3: // both are non-null
            return (aTexture->getNativeHandle() == _states.texture->getNativeHandle());

        default: // impossible
            HG_UNREACHABLE();
        }
    }

    bool _isShaderCompatibleWithCurrentBranch(const Shader* aShader) const {
        return _states.shader == aShader;
    }

    bool _isTransformCompatibleWithCurrentBatch(const Transform* aTransform) const {
        const auto selector =
            static_cast<int>(!!_states.transform) | (static_cast<int>(!!aTransform) << 1);

        switch (selector) {
        case 0: // both are null
            return true;

        case 1: // aTransform is null
            return _states.transform->eq(nullptr);

        case 2: // _states.transform is null
        case 3: // both are non-null
            return aTransform->eq(nullptr);

        default: // impossible
            HG_UNREACHABLE();
        }
    }
};

} // namespace detail
} // namespace uwga
HOBGOBLIN_NAMESPACE_END

#include <Hobgoblin/Private/Pmacro_undef.hpp>
#include <Hobgoblin/Private/Short_namespace.hpp>

#endif // !UHOBGOBLIN_UWGA_PRIVATE_DRAW_BATCHING_DECORATOR_FNL_HPP
