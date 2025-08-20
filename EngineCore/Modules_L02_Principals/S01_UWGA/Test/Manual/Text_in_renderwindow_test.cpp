// Copyright 2025 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#include <Hobgoblin/UWGA.hpp>

#include <Hobgoblin/Logging.hpp>

namespace jbatnozic {
namespace hobgoblin {
namespace uwga {
namespace {

#define LOG_ID "Hobgoblin.UWGA"

void RunTextInRenderWindowTest() {
    auto system = CreateGraphicsSystem("SFML");
    auto window = system->createRenderWindow(800, 800, WindowStyle::DEFAULT, "UWGA.ManualTest");
    auto view   = window->createDefaultView();
    auto text   = system->createText(system->getBuiltinFont(BuiltInFont::TITILLIUM_REGULAR),
                                   std::string("Test-Text"));

    text->setFillColor(COLOR_RED);
    text->setOutlineThickness(2.f);
    text->setOutlineColor(COLOR_YELLOW);

    window->setFramerateLimit(60);

    //! The main purpose of anchors is to make the graphics engine able to deal with
    //! huge coordinates, so let's check if it works.
    const math::Vector2d VERY_FAR_AWAY_ANCHOR = {10'000'000'000.0, 10'000'000'000.0};

    math::Vector2d viewAnchor = VERY_FAR_AWAY_ANCHOR;
    math::Vector2d textAnchor = VERY_FAR_AWAY_ANCHOR;
    math::Vector2f textOffset;

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
            const auto lr = (double)in::CheckPressedPK(in::PK_L) - (double)in::CheckPressedPK(in::PK_J);
            const auto ud = (double)in::CheckPressedPK(in::PK_K) - (double)in::CheckPressedPK(in::PK_I);
            viewAnchor.x += lr * 2.0;
            viewAnchor.y += ud * 2.0;
            view->setAnchor(viewAnchor);
        }

        {
            const auto lr = (float)in::CheckPressedPK(in::PK_D) - (float)in::CheckPressedPK(in::PK_A);
            const auto ud = (float)in::CheckPressedPK(in::PK_S) - (float)in::CheckPressedPK(in::PK_W);
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
                textAnchor.x += lr * 2.0;
                textAnchor.y += ud * 2.0;
            } else {
                textOffset.x += lr * 2.0;
                textOffset.y += ud * 2.0;
            }
        }

        window->clear(COLOR_BLACK);
        window->setView(*view);

        // Draw the text
        {
            text->setAnchor(textAnchor);
            text->setPosition(textOffset);
            window->draw(*text);
        }

        window->display();
    }
}

} // namespace
} // namespace uwga
} // namespace hobgoblin
} // namespace jbatnozic

void RunTextInRenderWindowTest(int, const char**) {
    jbatnozic::hobgoblin::uwga::RunTextInRenderWindowTest();
}
