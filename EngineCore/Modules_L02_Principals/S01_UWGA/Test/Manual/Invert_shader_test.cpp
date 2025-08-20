// Copyright 2025 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#include <Hobgoblin/UWGA.hpp>

namespace jbatnozic {
namespace hobgoblin {
namespace uwga {
namespace {

void RunInvertShaderTest() {
    auto system = CreateGraphicsSystem("SFML");

    SpriteLoader loader{*system};
    loader.startTexture(2048, 2048)
        ->addSprite("troidco", std::filesystem::path{HG_TEST_ASSET_DIR} / "Troidco.png")
        ->finalize(uwga::TexturePackingHeuristic::BEST_AREA_FIT);

    auto troidcoSpr = loader.getBlueprint("troidco").spr();

    auto shader = system->createGLSLShader(
        std::filesystem::path{HG_TEST_ASSET_DIR} / "Invert_shader_vertex.txt",
        std::filesystem::path{HG_TEST_ASSET_DIR} / "Invert_shader_fragment.txt");

    auto window = system->createRenderWindow(800, 800, WindowStyle::DEFAULT, "UWGA.ManualTest");
    window->setFramerateLimit(60);

    auto view = system->createView();
    view->setSize({1024.f, 1024.f});
    view->setCenter({512.f, 512.f});
    view->setViewport({0.f, 0.f, 1.f, 1.f});
    window->setView(*view);

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
        window->draw(troidcoSpr, {.shader = shader.get()});
        window->display();
    }
}

} // namespace
} // namespace uwga
} // namespace hobgoblin
} // namespace jbatnozic

void RunInvertShaderTest(int, const char**) {
    jbatnozic::hobgoblin::uwga::RunInvertShaderTest();
}
