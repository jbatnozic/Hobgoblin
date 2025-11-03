// Copyright 2025 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#include <Hobgoblin/UWGA.hpp>

#include <Hobgoblin/Logging.hpp>

#include <array>

namespace jbatnozic {
namespace hobgoblin {
namespace uwga {
namespace {

void RunRenderTextureInRenderWindowTest() {
    auto system = CreateGraphicsSystem("SFML");
    auto window = system->createRenderWindow(800, 800, WindowStyle::DEFAULT, "UWGA.ManualTest");

    auto texture = system->createRenderTexture(128, 128, false);
    {
        texture->clear(COLOR_LIME_GREEN);

        // clang-format off
        const std::array<Vertex, 5> diamondVertices = {
            Vertex{ .position = {16.f,  0.f}, .color = COLOR_RED},
            Vertex{ .position = { 0.f, 16.f}, .color = COLOR_RED},
            Vertex{ .position = {16.f, 32.f}, .color = COLOR_RED},
            Vertex{ .position = {32.f, 16.f}, .color = COLOR_RED},
            Vertex{ .position = {16.f,  0.f}, .color = COLOR_RED}
        };
        // clang-format on

        texture->draw(diamondVertices.data(),
                      stopz(diamondVertices.size()),
                      PrimitiveType::TRIANGLE_STRIP,
                      {4.0, 4.0});

        texture->draw(diamondVertices.data(),
                      stopz(diamondVertices.size()),
                      PrimitiveType::TRIANGLE_STRIP,
                      {18.0, 24.0});

        texture->draw(diamondVertices.data(),
                      stopz(diamondVertices.size()),
                      PrimitiveType::TRIANGLE_STRIP,
                      {80.0, 64.0});

        texture->draw(diamondVertices.data(),
                      stopz(diamondVertices.size()),
                      PrimitiveType::TRIANGLE_STRIP,
                      {100.0, 100.0});

        texture->display();
    }

    window->setFramerateLimit(60);

    // clang-format off
    const std::array<Vertex, 5> boxVertices = {
        Vertex{ .position = {  0.f,   0.f}, .color = COLOR_WHITE, .texCoords = {  0.f,   0.f}},
        Vertex{ .position = {  0.f, 128.f}, .color = COLOR_WHITE, .texCoords = {  0.f, 128.f}},
        Vertex{ .position = {128.f, 128.f}, .color = COLOR_WHITE, .texCoords = {128.f, 128.f}},
        Vertex{ .position = {128.f,   0.f}, .color = COLOR_WHITE, .texCoords = {128.f,   0.f}},
        Vertex{ .position = {  0.f,   0.f}, .color = COLOR_WHITE, .texCoords = {  0.f,   0.f}}
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

void RunRenderTextureInRenderWindowTest(const std::vector<const char*>& /*aArgs*/) {
    jbatnozic::hobgoblin::uwga::RunRenderTextureInRenderWindowTest();
}
