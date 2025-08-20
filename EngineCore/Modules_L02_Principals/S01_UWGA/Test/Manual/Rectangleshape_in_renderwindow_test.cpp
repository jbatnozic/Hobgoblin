// Copyright 2025 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#include <Hobgoblin/UWGA.hpp>

namespace jbatnozic {
namespace hobgoblin {
namespace uwga {
namespace {

void RunRectangleShapeInRenderWindowTest() {
    auto system = CreateGraphicsSystem("SFML");
    auto window = system->createRenderWindow(800, 800, WindowStyle::DEFAULT, "UWGA.ManualTest");
    auto view   = window->createDefaultView();

    window->setFramerateLimit(60);

    RectangleShape rectangle1{
        *system,
        {64.f, 64.f}
    };
    rectangle1.setOrigin({32.f, 32.f});
    rectangle1.setFillColor(COLOR_RED);
    rectangle1.setOutlineThickness(2.f);
    rectangle1.setOutlineColor(COLOR_LIME);

    // Test copying
    RectangleShape rectangle2 = rectangle1;

    // Test moving
    RectangleShape rectangle3 = std::move(rectangle2);
    rectangle3.setFillColor(COLOR_BLUE);
    rectangle3.setOutlineColor(COLOR_AQUAMARINE);

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
            rectangle1.move(lr * 4.f, ud * 4.f);
            rectangle3.move(lr * 4.f, ud * 4.f);
        }

        window->clear(COLOR_BLACK);
        window->setView(*view);
        window->draw(rectangle1, {.transform = transform.get()});
        window->draw(rectangle3);
        window->display();
    }
}

} // namespace
} // namespace uwga
} // namespace hobgoblin
} // namespace jbatnozic

void RunRectangleShapeInRenderWindowTest(int, const char**) {
    jbatnozic::hobgoblin::uwga::RunRectangleShapeInRenderWindowTest();
}
