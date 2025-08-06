// Copyright 2024 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#include <Hobgoblin/UWGA.hpp>

#include <Hobgoblin/Logging.hpp>

#include <array>

namespace jbatnozic {
namespace hobgoblin {
namespace uwga {
namespace {

void RunTexturedVerticesInRenderWindowTest() {
    auto system = CreateRenderSystem("SFML");
    auto window = system->createRenderWindow(800, 800, WindowStyle::DEFAULT, "UWGA.ManualTest");

    auto image = system->createImage(32, 32);
    for (PZInteger y = 0; y < 32; ++y) {
        Color colors[] = {COLOR_RED, COLOR_GREEN, COLOR_BLUE, COLOR_YELLOW};
        for (PZInteger x = 0; x < 32; ++x) {
            image->setPixel(x, y, colors[(y / 4) % 4]);
        }
    }

    auto texture = system->createTexture(*image);

    window->setFramerateLimit(60);

    // clang-format off
    const std::array<Vertex, 5> boxVertices = {
        Vertex{ .position = {  0.f,   0.f}, .color = COLOR_WHITE, .texCoords = { 0.f,  0.f}},
        Vertex{ .position = {  0.f, 128.f}, .color = COLOR_WHITE, .texCoords = { 0.f, 32.f}},
        Vertex{ .position = {128.f, 128.f}, .color = COLOR_WHITE, .texCoords = {32.f, 32.f}},
        Vertex{ .position = {128.f,   0.f}, .color = COLOR_WHITE, .texCoords = {32.f,  0.f}},
        Vertex{ .position = {  0.f,   0.f}, .color = COLOR_WHITE, .texCoords = { 0.f,  0.f}}
    };
    // clang-format on

    math::Vector2d boxAnchor{0.0, 0.0};

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

        {
            const auto lr =
                (float)in::CheckPressedPK(in::PK_RIGHT) - (float)in::CheckPressedPK(in::PK_LEFT);
            const auto ud =
                (float)in::CheckPressedPK(in::PK_DOWN) - (float)in::CheckPressedPK(in::PK_UP);

            boxAnchor.x += lr * 2.0;
            boxAnchor.y += ud * 2.0;
        }

        window->clear(COLOR_BLACK);

        // Draw the box
        {
            std::array<Vertex, 5> vertices;
            for (std::size_t i = 0; i < 5; i += 1) {
                vertices[i] = boxVertices[i];
            }
            window->draw(vertices.data(),
                         stopz(vertices.size()),
                         PrimitiveType::TRIANGLE_STRIP,
                         boxAnchor,
                         {.texture = texture.get()});
        }

        window->display();
    }
}

} // namespace
} // namespace uwga
} // namespace hobgoblin
} // namespace jbatnozic

void RunTexturedVerticesInRenderWindowTest(int, const char**) {
    jbatnozic::hobgoblin::uwga::RunTexturedVerticesInRenderWindowTest();
}
