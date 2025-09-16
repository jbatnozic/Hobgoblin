// Copyright 2025 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#include <Hobgoblin/HGExcept.hpp>
#include <Hobgoblin/Logging.hpp>
#include <Hobgoblin/UWGA.hpp>
#include <Hobgoblin/Utility/Time_utils.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>

static constexpr auto LOG_ID = "Hobgoblin.UWGA.PerformanceTest";

#include <Hobgoblin/Private/Pmacro_define.hpp>

HOBGOBLIN_NAMESPACE_BEGIN
namespace {

#define BATCHING_DISABLE 0
#define BATCHING_PARTIAL 1
#define BATCHING_FULL    2

std::chrono::microseconds FuncSFML(int aIterCount) {
    sf::RenderWindow window;
    window.create(sf::VideoMode{800, 800}, "SFML Window");

    sf::View view;
    view.setSize(1024, 1024);
    view.setCenter(512, 512);
    view.setViewport({0.f, 0.f, 1.f, 1.f});
    window.setView(view);

    sf::Image image;
    HG_HARD_ASSERT(image.loadFromFile(std::string{HG_TEST_ASSET_DIR} + "/w95icon.png"));

    sf::Texture texture;
    HG_HARD_ASSERT(texture.loadFromImage(image));

    sf::Sprite sprite{texture};

    const auto rows    = static_cast<int>(1024 / sprite.getLocalBounds().height);
    const auto columns = static_cast<int>(1024 / sprite.getLocalBounds().width);

    util::Stopwatch stopwatch;

    for (int count = 0; count < aIterCount; ++count) {
        sf::Event ev;
        while (window.pollEvent(ev)) { /* Ignore */
        }

        window.clear();

        for (int i = 0; i < rows; ++i) {
            for (int t = 0; t < columns; ++t) {
                const auto bounds = sprite.getLocalBounds();
                sprite.setPosition(bounds.width * t, bounds.height * i);
                window.draw(sprite);
            }
        }

        window.display();
    }

    auto timeElapsed = stopwatch.getElapsedTime<std::chrono::microseconds>();
    return timeElapsed;
}

std::chrono::microseconds FuncUWGA(int aIterCount, int aBatchingChoice) {
    auto system = uwga::CreateGraphicsSystem("SFML");

    std::unique_ptr<uwga::RenderWindow> window;
    if (aBatchingChoice == BATCHING_DISABLE) {
        window = system->createRenderWindow(800, 800, uwga::WindowStyle::DEFAULT, "UWGA Window");
    } else {
        window = system->createRenderWindow(
            uwga::BatchingConfig{.strategy = uwga::BatchingConfig::Strategy::FAST_N_LOOSE},
            800,
            800,
            uwga::WindowStyle::DEFAULT,
            "UWGA Window");
    }

    auto view = window->createDefaultView();
    view->setSize(1024, 1024);
    view->setCenter(512, 512);
    view->setViewport({0.f, 0.f, 1.f, 1.f});
    window->setView(*view);

    auto texture = system->createTexture(std::string{HG_TEST_ASSET_DIR} + "/w95icon.png");

    uwga::Sprite sprite{*texture};

    const auto rows    = static_cast<int>(1024 / sprite.getLocalBounds().h);
    const auto columns = static_cast<int>(1024 / sprite.getLocalBounds().w);

    util::Stopwatch stopwatch;

    for (int count = 0; count < aIterCount; ++count) {
        uwga::WindowEvent ev;
        while (window->pollEvent(ev)) { /* Ignore */
        }

        window->clear();

        for (int i = 0; i < rows; ++i) {
            if (aBatchingChoice == BATCHING_PARTIAL) {
                window->flush();
            }

            for (int t = 0; t < columns; ++t) {
                const auto bounds = sprite.getLocalBounds();
                sprite.setPosition(bounds.w * t, bounds.h * i);
                window->draw(sprite);
            }
        }

        window->flush();
        window->display();
    }

    auto timeElapsed = stopwatch.getElapsedTime<std::chrono::microseconds>();
    return timeElapsed;
}

} // namespace
HOBGOBLIN_NAMESPACE_END

void RunSpriteDrawingPerfTest(int, const char**) {
    const int iterCount = 1800;

    {
        auto t1 = jbatnozic::hobgoblin::FuncSFML(iterCount);
        HG_LOG_INFO(LOG_ID, "SFML test took {}ms.", t1.count() / 1000.0);
    }
    {
        auto t2 = jbatnozic::hobgoblin::FuncUWGA(iterCount, BATCHING_DISABLE);
        HG_LOG_INFO(LOG_ID, "UWGA (no batching) test took {}ms.", t2.count() / 1000.0);
    }
    {
        auto t3 = jbatnozic::hobgoblin::FuncUWGA(iterCount, BATCHING_PARTIAL);
        HG_LOG_INFO(LOG_ID, "UWGA (limited batching) test took {}ms.", t3.count() / 1000.0);
    }
    {
        auto t4 = jbatnozic::hobgoblin::FuncUWGA(iterCount, BATCHING_FULL);
        HG_LOG_INFO(LOG_ID, "UWGA (full batching) test took {}ms.", t4.count() / 1000.0);
    }
}

#include <Hobgoblin/Private/Pmacro_undef.hpp>
