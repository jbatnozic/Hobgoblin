// Copyright 2025 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#include <Hobgoblin/UWGA.hpp>
#include <Hobgoblin/UWGA/Private/Draw_batching_decorator_fnl.hpp>

#include "Canvas_mock.hpp"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <array>
#include <memory>

namespace jbatnozic {
namespace hobgoblin {
namespace uwga {
namespace {

// MARK: Fixture

class DelegatingCanvas : public Canvas {
public:
    explicit DelegatingCanvas(Canvas& aDelegate)
        : _delegate{aDelegate} {}

    const System& getSystem() const override {
        return _delegate.getSystem();
    }

    math::Vector2pz getSize() const override {
        return _delegate.getSize();
    }

    void setView(const View& aView) override {
        _delegate.setView(aView);
    }

    const View& getView() const override {
        return _delegate.getView();
    }

    void setDefaultView() override {
        _delegate.setDefaultView();
    }

    std::unique_ptr<View> createDefaultView() const override {
        return _delegate.createDefaultView();
    }

    math::Rectangle<int> viewportToPixels(const View& aView) const override {
        return _delegate.viewportToPixels(aView);
    }

    void clear(Color aColor = COLOR_BLACK) override {
        _delegate.clear(aColor);
    }

    void draw(const Vertex*       aVertices,
              PZInteger           aVertexCount,
              PrimitiveType       aPrimitiveType,
              math::Vector2d      aAnchor,
              const RenderStates& aStates = RENDER_STATES_DEFAULT) override {
        _delegate.draw(aVertices, aVertexCount, aPrimitiveType, aAnchor, aStates);
    }

    void flush() override {
        _delegate.flush();
    }

    const PerformanceCounters& getPerformanceCounters() const override {
        return _delegate.getPerformanceCounters();
    }

    void resetPerformanceCounters() override {
        _delegate.resetPerformanceCounters();
    }

    PerformanceCounters getAndResetPerformanceCounters() override {
        return _delegate.getAndResetPerformanceCounters();
    }

private:
    Canvas& _delegate;
};

class FastNLooseDrawBatchingDecoratorTest : public testing::Test {
public:
    void SetUp() override {
        _graphicsSystem = CreateGraphicsSystem("SFML");

        _texture1 = _graphicsSystem->createTexture(128, 128);
        _texture2 = _graphicsSystem->createTexture(128, 128);

        _transform1 = _graphicsSystem->createTransform();
        _transform1->setToIdentity();
        _transform2 = _transform1->clone();
        _transform2->scale({2.f, 2.f});

        _shader1 = _graphicsSystem->createGLSLShader();
        _shader2 = _graphicsSystem->createGLSLShader();

        ASSERT_TRUE(_graphicsSystem && _texture1 && _texture2 && _transform1 && _transform2 &&
                    _shader1 && _shader2);
    }

    void TearDown() override {
        _verifyAndClearCtrlCanvasExpectations();
    }

protected:
    // General graphics elements
    std::shared_ptr<System>    _graphicsSystem;
    std::unique_ptr<Texture>   _texture1;
    std::unique_ptr<Texture>   _texture2;
    std::unique_ptr<Transform> _transform1;
    std::unique_ptr<Transform> _transform2;
    std::unique_ptr<Shader>    _shader1;
    std::unique_ptr<Shader>    _shader2;

    std::array<Vertex, 5> _vertices = {
        Vertex{    .position = {0.f, 0.f}, .color = COLOR_WHITE,   .texCoords = {0.f, 0.f}},
        Vertex{  .position = {0.f, 128.f}, .color = COLOR_WHITE,  .texCoords = {0.f, 32.f}},
        Vertex{.position = {128.f, 128.f}, .color = COLOR_WHITE, .texCoords = {32.f, 32.f}}
    };

    // Canvas (and its mock)
    testing::StrictMock<MockCanvas>                           _controlCanvas;
    detail::FastNLooseDrawBatchingDecorator<DelegatingCanvas> _canvas{_controlCanvas};

    void _verifyAndClearCtrlCanvasExpectations() {
        testing::Mock::VerifyAndClearExpectations(&_controlCanvas);
    }
};

// MARK: Test cases

using testing::_;

TEST_F(FastNLooseDrawBatchingDecoratorTest, TrianglesDrawnWithDefaultRenderStates) {
    _canvas.draw(_vertices.data(),
                 stopz(_vertices.size()),
                 PrimitiveType::TRIANGLES,
                 math::Vector2d{0.0, 0.0});
    _verifyAndClearCtrlCanvasExpectations();

    _canvas.draw(_vertices.data(),
                 stopz(_vertices.size()),
                 PrimitiveType::TRIANGLES,
                 math::Vector2d{0.0, 0.0});
    _verifyAndClearCtrlCanvasExpectations();

    EXPECT_CALL(
        _controlCanvas,
        draw(_, stopz(2 * _vertices.size()), PrimitiveType::TRIANGLES, math::Vector2d{0.0, 0.0}, _));
    EXPECT_CALL(_controlCanvas, flush());

    _canvas.flush();
    _verifyAndClearCtrlCanvasExpectations();

    const auto pc = _canvas.getAndResetPerformanceCounters();
    ASSERT_EQ(pc.vcallCount, 2);
    ASSERT_EQ(pc.ucallCount, 1);
    ASSERT_EQ(pc.maxAggregation, 2);
}

TEST_F(FastNLooseDrawBatchingDecoratorTest, TrianglesDrawn_ChangingTransformBreaksBatch) {
    _canvas.draw(_vertices.data(),
                 stopz(_vertices.size()),
                 PrimitiveType::TRIANGLES,
                 math::Vector2d{0.0, 0.0});
    _verifyAndClearCtrlCanvasExpectations();

    EXPECT_CALL(_controlCanvas,
                draw(_, stopz(_vertices.size()), PrimitiveType::TRIANGLES, math::Vector2d{0.0, 0.0}, _));

    _canvas.draw(_vertices.data(),
                 stopz(_vertices.size()),
                 PrimitiveType::TRIANGLES,
                 math::Vector2d{0.0, 0.0},
                 {.transform = _transform2.get()});
    _verifyAndClearCtrlCanvasExpectations();

    {
        const auto pc = _canvas.getPerformanceCounters();
        ASSERT_EQ(pc.vcallCount, 2);
        ASSERT_EQ(pc.ucallCount, 1);
        ASSERT_EQ(pc.maxAggregation, 1);
    }

    EXPECT_CALL(_controlCanvas,
                draw(_, stopz(_vertices.size()), PrimitiveType::TRIANGLES, math::Vector2d{0.0, 0.0}, _));
    EXPECT_CALL(_controlCanvas, flush());

    _canvas.flush();
    _verifyAndClearCtrlCanvasExpectations();

    {
        const auto pc = _canvas.getAndResetPerformanceCounters();
        ASSERT_EQ(pc.vcallCount, 2);
        ASSERT_EQ(pc.ucallCount, 2);
        ASSERT_EQ(pc.maxAggregation, 1);
    }
}

TEST_F(FastNLooseDrawBatchingDecoratorTest, TrianglesDrawn_ChangingTextureBreaksBatch) {
    _canvas.draw(_vertices.data(),
                 stopz(_vertices.size()),
                 PrimitiveType::TRIANGLES,
                 math::Vector2d{0.0, 0.0},
                 {.texture = _texture1.get()});
    _verifyAndClearCtrlCanvasExpectations();

    EXPECT_CALL(_controlCanvas,
                draw(_, stopz(_vertices.size()), PrimitiveType::TRIANGLES, math::Vector2d{0.0, 0.0}, _));

    _canvas.draw(_vertices.data(),
                 stopz(_vertices.size()),
                 PrimitiveType::TRIANGLES,
                 math::Vector2d{0.0, 0.0},
                 {.texture = _texture2.get()});
    _verifyAndClearCtrlCanvasExpectations();

    {
        const auto pc = _canvas.getPerformanceCounters();
        ASSERT_EQ(pc.vcallCount, 2);
        ASSERT_EQ(pc.ucallCount, 1);
        ASSERT_EQ(pc.maxAggregation, 1);
    }

    EXPECT_CALL(_controlCanvas,
                draw(_, stopz(_vertices.size()), PrimitiveType::TRIANGLES, math::Vector2d{0.0, 0.0}, _));
    EXPECT_CALL(_controlCanvas, flush());

    _canvas.flush();
    _verifyAndClearCtrlCanvasExpectations();

    {
        const auto pc = _canvas.getAndResetPerformanceCounters();
        ASSERT_EQ(pc.vcallCount, 2);
        ASSERT_EQ(pc.ucallCount, 2);
        ASSERT_EQ(pc.maxAggregation, 1);
    }
}

TEST_F(FastNLooseDrawBatchingDecoratorTest, TrianglesDrawn_ChangingBlendModeBreaksBatch) {
    _canvas.draw(_vertices.data(),
                 stopz(_vertices.size()),
                 PrimitiveType::TRIANGLES,
                 math::Vector2d{0.0, 0.0},
                 {.blendMode = BLEND_NORMAL});
    _verifyAndClearCtrlCanvasExpectations();

    EXPECT_CALL(_controlCanvas,
                draw(_, stopz(_vertices.size()), PrimitiveType::TRIANGLES, math::Vector2d{0.0, 0.0}, _));

    _canvas.draw(_vertices.data(),
                 stopz(_vertices.size()),
                 PrimitiveType::TRIANGLES,
                 math::Vector2d{0.0, 0.0},
                 {.blendMode = BLEND_ADD});
    _verifyAndClearCtrlCanvasExpectations();

    {
        const auto pc = _canvas.getPerformanceCounters();
        ASSERT_EQ(pc.vcallCount, 2);
        ASSERT_EQ(pc.ucallCount, 1);
        ASSERT_EQ(pc.maxAggregation, 1);
    }

    EXPECT_CALL(_controlCanvas,
                draw(_, stopz(_vertices.size()), PrimitiveType::TRIANGLES, math::Vector2d{0.0, 0.0}, _));
    EXPECT_CALL(_controlCanvas, flush());

    _canvas.flush();
    _verifyAndClearCtrlCanvasExpectations();

    {
        const auto pc = _canvas.getAndResetPerformanceCounters();
        ASSERT_EQ(pc.vcallCount, 2);
        ASSERT_EQ(pc.ucallCount, 2);
        ASSERT_EQ(pc.maxAggregation, 1);
    }
}

TEST_F(FastNLooseDrawBatchingDecoratorTest, TrianglesDrawn_ChangingShaderBreaksBatch) {
    _canvas.draw(_vertices.data(),
                 stopz(_vertices.size()),
                 PrimitiveType::TRIANGLES,
                 math::Vector2d{0.0, 0.0},
                 {.shader = _shader1.get()});
    _verifyAndClearCtrlCanvasExpectations();

    EXPECT_CALL(_controlCanvas,
                draw(_, stopz(_vertices.size()), PrimitiveType::TRIANGLES, math::Vector2d{0.0, 0.0}, _));

    _canvas.draw(_vertices.data(),
                 stopz(_vertices.size()),
                 PrimitiveType::TRIANGLES,
                 math::Vector2d{0.0, 0.0},
                 {.shader = _shader2.get()});
    _verifyAndClearCtrlCanvasExpectations();

    {
        const auto pc = _canvas.getPerformanceCounters();
        ASSERT_EQ(pc.vcallCount, 2);
        ASSERT_EQ(pc.ucallCount, 1);
        ASSERT_EQ(pc.maxAggregation, 1);
    }

    EXPECT_CALL(_controlCanvas,
                draw(_, stopz(_vertices.size()), PrimitiveType::TRIANGLES, math::Vector2d{0.0, 0.0}, _));
    EXPECT_CALL(_controlCanvas, flush());

    _canvas.flush();
    _verifyAndClearCtrlCanvasExpectations();

    {
        const auto pc = _canvas.getAndResetPerformanceCounters();
        ASSERT_EQ(pc.vcallCount, 2);
        ASSERT_EQ(pc.ucallCount, 2);
        ASSERT_EQ(pc.maxAggregation, 1);
    }
}

} // namespace
} // namespace uwga
} // namespace hobgoblin
} // namespace jbatnozic
