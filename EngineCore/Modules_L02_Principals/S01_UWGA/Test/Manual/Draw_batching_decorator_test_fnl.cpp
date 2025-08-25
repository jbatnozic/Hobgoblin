// Copyright 2025 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#include <Hobgoblin/UWGA.hpp>
#include <Hobgoblin/UWGA/Private/Draw_batching_decorator_fnl.hpp>

#include <Hobgoblin/Logging.hpp>

#include <array>

namespace jbatnozic {
namespace hobgoblin {
namespace uwga {
namespace {

void RunFastNLooseDrawBatchingDecoratorTest() {
    auto system = CreateGraphicsSystem("SFML");
    auto window =
        system->createRenderWindow(BatchingConfig{.strategy = BatchingConfig::Strategy::FAST_N_LOOSE},
                                   800,
                                   800,
                                   WindowStyle::DEFAULT,
                                   "UWGA.ManualTest");

    auto shader = system->createGLSLShader(
        std::filesystem::path{HG_TEST_ASSET_DIR} / "Invert_shader_vertex.txt",
        std::filesystem::path{HG_TEST_ASSET_DIR} / "Invert_shader_fragment.txt");

    auto image = system->createImage(32, 32);
    for (PZInteger y = 0; y < 32; ++y) {
        Color colors[] = {COLOR_RED, COLOR_GREEN, COLOR_BLUE, COLOR_YELLOW};
        for (PZInteger x = 0; x < 32; ++x) {
            if (x == 0 || y == 0 || x == 31 || y == 31) {
                image->setPixel(x, y, COLOR_BEIGE);
            } else {
                image->setPixel(x, y, colors[(y / 4) % 4]);
            }
        }
    }

    auto texture = system->createTexture(*image);

    window->setFramerateLimit(60);

    // clang-format off
    const std::array<Vertex, 6> boxVertices = {
        Vertex{ .position = {  0.f,   0.f}, .color = COLOR_WHITE, .texCoords = { 0.f,  0.f}},
        Vertex{ .position = {128.f,   0.f}, .color = COLOR_WHITE, .texCoords = {32.f,  0.f}},
        Vertex{ .position = {  0.f, 128.f}, .color = COLOR_WHITE, .texCoords = { 0.f, 32.f}},
        Vertex{ .position = {128.f,   0.f}, .color = COLOR_WHITE, .texCoords = {32.f,  0.f}},
        Vertex{ .position = {128.f, 128.f}, .color = COLOR_WHITE, .texCoords = {32.f, 32.f}},
        Vertex{ .position = {  0.f, 128.f}, .color = COLOR_WHITE, .texCoords = { 0.f, 32.f}}
    };
    // clang-format on

    while (true) {
        WindowEvent ev;
        while (window && window->pollEvent(ev)) {
            ev.visit([&window](WindowEvent::Closed) {
                window.reset();
            });
        }
        if (!window) {
            break;
        }

        window->clear(COLOR_BLACK);

        // Draw the box
        RenderStates states{.texture = texture.get()};
        for (int i = 0; i < 8; ++i) {
            math::Vector2d anchor{i * 64.0, i * 64.0};

            if (i == 4) {
                states.shader = shader.get();
            } else {
                states.shader = nullptr;
            }

            std::array<Vertex, 6> vertices = boxVertices;
            window->draw(vertices.data(),
                         stopz(vertices.size()),
                         PrimitiveType::TRIANGLES,
                         anchor,
                         states);
        }

        window->display();

        const auto pc1 = window->getAndResetPerformanceCounters();
        const auto pc2 = window->getAndResetPerformanceCounters();
    }
}

} // namespace
} // namespace uwga
} // namespace hobgoblin
} // namespace jbatnozic

void RunFastNLooseDrawBatchingDecoratorTest(int, const char**) {
    jbatnozic::hobgoblin::uwga::RunFastNLooseDrawBatchingDecoratorTest();
}
