// Copyright 2024 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#include <Hobgoblin/UnifiedGraphicsEngine.hpp>

#include <Hobgoblin/Logging.hpp>

#include <array>

namespace jbatnozic {
namespace hobgoblin {
namespace uge {
namespace {

void RunTransformInRenderWindowTest() {
    auto system    = CreateRenderSystem("SFML");
    auto window    = system->createRenderWindow(800, 800, WindowStyle::DEFAULT, "UWGA.ManualTest");
    auto view      = system->createDefaultView(*window);
    auto transform = system->createTransform();

    window->setFramerateLimit(60);

    // clang-format off
    const std::array<Vertex, 5> boxVertices = {
        Vertex{ .position = { 0.f,  0.f}, .color = COLOR_YELLOW, .texCoords = {}},
        Vertex{ .position = { 0.f, 32.f}, .color = COLOR_RED,    .texCoords = {}},
        Vertex{ .position = {32.f, 32.f}, .color = COLOR_BLUE,   .texCoords = {}},
        Vertex{ .position = {32.f,  0.f}, .color = COLOR_GREEN,  .texCoords = {}},
        Vertex{ .position = { 0.f,  0.f}, .color = COLOR_YELLOW, .texCoords = {}}
    };
    const std::array<Vertex, 5> gridVertices = {
        Vertex{ .position = {-1024.f,     0.f}, .color = COLOR_YELLOW, .texCoords = {}},
        Vertex{ .position = {+1024.f,     0.f}, .color = COLOR_GREEN,  .texCoords = {}},
        Vertex{ .position = {    0.f, -1024.f}, .color = COLOR_RED,    .texCoords = {}},
        Vertex{ .position = {    0.f, +1024.f}, .color = COLOR_BLUE,   .texCoords = {}},
    };
    // clang-format on

    math::Vector2d viewAnchor;
    math::Vector2d boxAnchor;
    math::Vector2f boxOffset;

    RenderStates states;
    // clang-format off
    transform->setTo3x3Matrix( 1.0f, 1.0f, 0.f,
                              -0.5f, 0.5f, 0.f,
                                0.f, 0.0f, 1.f);
    // clang-format on
    states.transform = transform.get();

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
            viewAnchor.x += lr * 2.0;
            viewAnchor.y += ud * 2.0;
            view->setAnchor(viewAnchor);
        }

        {
            const auto lr = (float)in::CheckPressedPK(in::PK_L) - (float)in::CheckPressedPK(in::PK_J);
            const auto ud = (float)in::CheckPressedPK(in::PK_K) - (float)in::CheckPressedPK(in::PK_I);
            view->setCenter(view->getCenter() + math::Vector2f{lr, ud});

            const auto rot = (float)in::CheckPressedPK(in::PK_Q) - (float)in::CheckPressedPK(in::PK_E);
            view->rotate(math::AngleF::fromDegrees(rot));
        }

        {
            const auto lr =
                (float)in::CheckPressedPK(in::PK_RIGHT) - (float)in::CheckPressedPK(in::PK_LEFT);
            const auto ud =
                (float)in::CheckPressedPK(in::PK_DOWN) - (float)in::CheckPressedPK(in::PK_UP);

            if (!in::CheckPressedPK(in::PK_LSHIFT)) {
                boxAnchor.x += lr * 2.0;
                boxAnchor.y += ud * 2.0;
            } else {
                boxOffset.x += lr * 2.0;
                boxOffset.y += ud * 2.0;
            }
        }

        window->clear(COLOR_BLACK);
        window->setView(*view);

        // Draw grid
        window->draw(gridVertices.data(), stopz(gridVertices.size()), PrimitiveType::LINES, {});

        // Draw the box
        {
            std::array<Vertex, 5> vertices;
            for (std::size_t i = 0; i < 5; i += 1) {
                vertices[i] = boxVertices[i];
                vertices[i].position += boxOffset;
            }
            window->draw(vertices.data(),
                         stopz(vertices.size()),
                         PrimitiveType::TRIANGLE_STRIP,
                         boxAnchor,
                         states);
        }

        window->display();
    }
}

} // namespace
} // namespace uge
} // namespace hobgoblin
} // namespace jbatnozic

void RunTransformInRenderWindowTest(int, const char**) {
    jbatnozic::hobgoblin::uge::RunTransformInRenderWindowTest();
}
