// Copyright 2025 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#include <Hobgoblin/UWGA.hpp>

namespace jbatnozic {
namespace hobgoblin {
namespace uwga {
namespace {

void RunCircleShapeInRenderWindowTest() {
    auto system = CreateRenderSystem("SFML");
    auto window = system->createRenderWindow(800, 800, WindowStyle::DEFAULT, "UWGA.ManualTest");
    auto view   = window->createDefaultView();

    window->setFramerateLimit(60);

    CircleShape circle1{*system, 32.f, 32};
    circle1.setOrigin({32.f, 32.f});
    circle1.setFillColor(COLOR_RED);
    circle1.setOutlineThickness(2.f);
    circle1.setOutlineColor(COLOR_LIME);

    // Test copying
    CircleShape circle2 = circle1;

    // Test moving
    CircleShape circle3 = std::move(circle2);
    circle3.setFillColor(COLOR_BLUE);
    circle3.setOutlineColor(COLOR_AQUAMARINE);

    auto transform = system->createTransform();
    transform->translate({64.f, 0.f});

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
            const auto lr = (float)in::CheckPressedPK(in::PK_D) - (float)in::CheckPressedPK(in::PK_A);
            const auto ud = (float)in::CheckPressedPK(in::PK_S) - (float)in::CheckPressedPK(in::PK_W);
            circle1.move(lr * 4.f, ud * 4.f);
            circle3.move(lr * 4.f, ud * 4.f);
        }

        window->clear(COLOR_BLACK);
        window->setView(*view);
        window->draw(circle1, {.transform = transform.get()});
        window->draw(circle3);
        window->display();
    }
}

} // namespace
} // namespace uwga
} // namespace hobgoblin
} // namespace jbatnozic

void RunCircleShapeInRenderWindowTest(int, const char**) {
    jbatnozic::hobgoblin::uwga::RunCircleShapeInRenderWindowTest();
}
