// Copyright 2025 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#pragma once

#include <Hobgoblin/UWGA/Canvas.hpp>

#include <gmock/gmock.h>

#include <Hobgoblin/Private/Pmacro_define.hpp>

HOBGOBLIN_NAMESPACE_BEGIN
namespace uwga {

class MockCanvas : public Canvas {
public:
    // Destructor
    ~MockCanvas() override = default;

    MOCK_METHOD(const System&, getSystem, (), (const, override));

    // Size
    MOCK_METHOD(math::Vector2pz, getSize, (), (const, override));

    // Views
    MOCK_METHOD(void, setView, (const View& aView), (override));
    MOCK_METHOD(const View&, getView, (), (const, override));
    MOCK_METHOD(void, setDefaultView, (), (override));
    MOCK_METHOD(std::unique_ptr<View>, createDefaultView, (), (const, override));
    MOCK_METHOD(math::Rectangle<int>, viewportToPixels, (const View& aView), (const, override));

    // Drawing
    MOCK_METHOD(void, clear, (Color aColor), (override));
    MOCK_METHOD(void,
                draw,
                (const Vertex*       aVertices,
                 PZInteger           aVertexCount,
                 PrimitiveType       aPrimitiveType,
                 math::Vector2d      aAnchor,
                 const RenderStates& aStates),
                (override));
    MOCK_METHOD(void, flush, (), (override));

    // Performance counters
    MOCK_METHOD(const PerformanceCounters&, getPerformanceCounters, (), (const, override));
    MOCK_METHOD(void, resetPerformanceCounters, (), (override));
    MOCK_METHOD(PerformanceCounters, getAndResetPerformanceCounters, (), (override));
};

} // namespace uwga
HOBGOBLIN_NAMESPACE_END

#include <Hobgoblin/Private/Pmacro_undef.hpp>
