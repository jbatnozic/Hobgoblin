// Copyright 2025 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#include <Hobgoblin/UWGA.hpp>

namespace jbatnozic {
namespace hobgoblin {
namespace uwga {
namespace {

void RunDefaultShaderTest() {
    auto system = CreateRenderSystem("SFML");

    SpriteLoader loader{*system};
    loader.startTexture(2048, 2048)
        ->addSprite("troidco", std::string{HG_TEST_ASSET_DIR} + "/Troidco.png")
        ->finalize(uwga::TexturePackingHeuristic::BEST_AREA_FIT);

    auto troidcoSpr = loader.getBlueprint("troidco").spr();

    const std::string VERTEX_SHADER = R"_(
        void main() {
            // transform the vertex position
            gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;

            // transform the texture coordinates
            gl_TexCoord[0] = gl_TextureMatrix[0] * gl_MultiTexCoord0;

            // forward the vertex color
            gl_FrontColor = gl_Color;
        }
        )_";

    const std::string FRAGMENT_SHADER = R"_(
        uniform sampler2D texture;

        void main() {
            // lookup the pixel in the texture
            vec4 pixel = texture2D(texture, gl_TexCoord[0].xy);

            // multiply it by the color
            gl_FragColor = gl_Color * pixel;
        }
        )_";

    auto shader = system->createGLSLShader(VERTEX_SHADER, FRAGMENT_SHADER);

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

void RunDefaultShaderTest(int, const char**) {
    jbatnozic::hobgoblin::uwga::RunDefaultShaderTest();
}
