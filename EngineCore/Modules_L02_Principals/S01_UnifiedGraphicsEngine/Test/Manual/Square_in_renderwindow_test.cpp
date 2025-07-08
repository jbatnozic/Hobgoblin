// Copyright 2024 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#include <Hobgoblin/UnifiedGraphicsEngine.hpp>

#include <Hobgoblin/Logging.hpp>

#include <array>

namespace jbatnozic {
namespace hobgoblin {
namespace uge {
namespace {

void RunSquareInRenderWindowTest() {
    auto system = CreateRenderSystem("SFML");
    auto window = system->createRenderWindow();
    auto view = system->createDefaultView(*window);

    window->setFramerateLimit(60);

    // clang-format off
    const std::array<Vertex, 5> vertices = {
        Vertex{ .position = { 0.f,  0.f}, .color = COLOR_RED, .texCoords = {}},
        Vertex{ .position = { 0.f, 32.f}, .color = COLOR_RED, .texCoords = {}},
        Vertex{ .position = {32.f, 32.f}, .color = COLOR_RED, .texCoords = {}},
        Vertex{ .position = {32.f,  0.f}, .color = COLOR_RED, .texCoords = {}},
        Vertex{ .position = { 0.f,  0.f}, .color = COLOR_RED, .texCoords = {}}
    };
    // clang-format on

    math::Vector2d viewAnchor;

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
            const auto lr = (double)in::CheckPressedPK(in::PK_D) - (double)in::CheckPressedPK(in::PK_A);
            const auto ud = (double)in::CheckPressedPK(in::PK_S) - (double)in::CheckPressedPK(in::PK_W);
            viewAnchor.x += lr;
            viewAnchor.y += ud;
            view->setAnchor(viewAnchor);

            const auto rot = (float)in::CheckPressedPK(in::PK_Q) - (float)in::CheckPressedPK(in::PK_E);
            view->rotate(math::AngleF::fromDegrees(rot));
        }

        window->clear(COLOR_BLACK);
        window->setView(*view);
        window->draw(vertices.data(), stopz(vertices.size()), PrimitiveType::TRIANGLE_STRIP, {});
        window->display();
    }
}

} // namespace
} // namespace uge
} // namespace hobgoblin
} // namespace jbatnozic

void RunSquareInRenderWindowTest(int, const char**) {
    jbatnozic::hobgoblin::uge::RunSquareInRenderWindowTest();
}
