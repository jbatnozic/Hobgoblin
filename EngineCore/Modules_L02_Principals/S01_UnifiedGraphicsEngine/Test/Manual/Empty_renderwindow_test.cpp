// Copyright 2024 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#include <Hobgoblin/UnifiedGraphicsEngine.hpp>

#include <Hobgoblin/Logging.hpp>

namespace jbatnozic {
namespace hobgoblin {
namespace uge {
namespace {

void RunEmptyRenderWindowTest() {
    auto system = CreateRenderSystem("SFML");
    auto window = system->createRenderWindow();

    window->setFramerateLimit(60);

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
        window->display();
    }
}

} // namespace
} // namespace uge
} // namespace hobgoblin
} // namespace jbatnozic

void RunEmptyRenderWindowTest(int, const char**) {
    jbatnozic::hobgoblin::uge::RunEmptyRenderWindowTest();
}
