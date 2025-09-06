// Copyright 2024 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#define HOBGOBLIN_SHORT_NAMESPACE
#include <Hobgoblin/Common.hpp>
#include <Hobgoblin/Format.hpp>
#include <Hobgoblin/HGExcept.hpp>
#include <Hobgoblin/Logging.hpp>
#include <Hobgoblin/Math.hpp>
#include <Hobgoblin/UWGA.hpp>
#include <Hobgoblin/Unicode.hpp>

#include <SPeMPE/SPeMPE.hpp>

#include <cstdlib>
#include <filesystem>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace uwga = hg::uwga;
namespace spe  = ::jbatnozic::spempe;

constexpr auto LOG_ID = "Antimony";

#define VIEW_GUI   0
#define VIEW_WORLD 1

class MasterLoader {
public:
    MasterLoader(const uwga::System&          aGraphicsSystem,
                 const std::filesystem::path& aDirectory,
                 const std::string&           aBaseSpriteName,
                 hg::PZInteger                aSpriteCount)
        : _loader{aGraphicsSystem} //
    {
        for (hg::PZInteger i = 0; i < aSpriteCount; i += 1) {
            std::filesystem::path specificSpriteName = {aBaseSpriteName + std::to_string(i) + ".png"};
            const auto&           texture            = _loader.startTexture(2048, 2048)
                                      ->addSprite(i, aDirectory / specificSpriteName)
                                      ->finalize(uwga::TexturePackingHeuristic::BEST_AREA_FIT);
            _textures.push_back(&texture);
        }
    }

    uwga::SpriteBlueprint getMasterBlueprint(hg::PZInteger aIndex) const {
        return _loader.getBlueprint(aIndex);
    }

private:
    static constexpr uwga::SpriteIdNumerical SPRITE_ID = 0;

    uwga::SpriteLoader                _loader;
    std::vector<const uwga::Texture*> _textures;
};

class Editor {
public:
    Editor(const MasterLoader& aMasterLoader, hg::PZInteger aMasterCount, hg::PZInteger aLerpCount)
        : _masterLoader{aMasterLoader} {
        _offsets.resize(static_cast<std::size_t>(aMasterCount * (aLerpCount + 1)));
    }

    void onLeftClick(hg::math::Vector2d aWorldPosition) {
        _origin = aWorldPosition;
    }

    void draw(spe::WindowManagerInterface& aWinMgr) {
        auto& canvas = aWinMgr.getActiveCanvas();

        // Draw master
        const auto blueprint = _masterLoader.getMasterBlueprint(_stage);
        auto       sprite    = blueprint.spr();
        canvas.draw(sprite);

        // Draw origin
    }

    void drawGui(spe::WindowManagerInterface& aWinMgr) {
        auto& grSystem = aWinMgr.getGraphicsSystem();
        auto& canvas   = aWinMgr.getActiveCanvas();

        const auto string = hg::UFormat(HG_UNIFMT("Stage: {} / {} \n"
                                                  "Origin: {}, {}"),
                                        _stage,
                                        _substage,
                                        (_origin) ? _origin->x : 0.f,
                                        (_origin) ? _origin->y : 0.f);

        const auto& font = grSystem.getBuiltinFont(uwga::BuiltInFont::TITILLIUM_REGULAR);
        auto        text = grSystem.createText(font, string);
        text->setPosition({32.f, 32.f});
        text->setFillColor(uwga::COLOR_RED);
        text->setStyle(uwga::Text::REGULAR);
        canvas.draw(*text);

        if (_origin.has_value()) {
            _drawCrosshairs(aWinMgr, *_origin, uwga::COLOR_RED);
        }
    }

    void advance() {
        _substage += 1;
        if (_substage >= static_cast<int>(Substage::SUBSTAGE_COUNT)) {
            _substage = 0;
            _stage += 1;
        }
    }

    void back() {}

private:
    const MasterLoader& _masterLoader;

    std::optional<hg::math::Vector2d>              _origin;
    std::vector<std::optional<hg::math::Vector2f>> _offsets;

    enum class Substage {
        PLACE_GROIN,
        PLACE_LEFT_ANKLE,
        PLACE_RIGHT_ANKLE,

        SUBSTAGE_COUNT
    };

    int _stage    = 0;
    int _substage = 0;

    void _drawCrosshairs(spe::WindowManagerInterface& aWinMgr,
                         hg::math::Vector2d           aCenter,
                         uwga::Color                  aColor) {
        auto&      canvas    = aWinMgr.getActiveCanvas();
        const auto screenPos = aWinMgr.mapCoordsToPixel({aCenter.x, aCenter.y});

        uwga::RectangleShape rect{canvas.getSystem()};
        rect.setFillColor(aColor);

        // Horizontal
        rect.setSize({(float)aWinMgr.getWindowSize().x, 1.f});
        rect.setPosition({0.f, (float)screenPos.y});
        canvas.draw(rect);

        // Vertical
        rect.setSize({1.f, (float)aWinMgr.getWindowSize().y});
        rect.setPosition({(float)screenPos.x, 1.f});
        canvas.draw(rect);
    }
};

class EditorDriver : public spe::NonstateObject {
public:
    EditorDriver(hg::QAO_InstGuard aInstGuard, int aExecutionPriority)
        : spe::NonstateObject{aInstGuard, SPEMPE_TYPEID_SELF, aExecutionPriority, "EditorDriver"} {}

    void init(Editor& aEditor) {
        _editor = &aEditor;
    }

    void _eventUpdate1() override {
        HG_HARD_ASSERT(_editor != nullptr);

        auto&       winMgr = ccomp<spe::WindowManagerInterface>();
        const auto& input  = winMgr.getInput();

        _editorZoom -= input.getVerticalMouseWheelScroll() * 0.1f;
        if (_editorZoom < 0.1f) {
            _editorZoom = 0.1f;
        }

        if (input.checkPressed(hg::in::MB_LEFT)) {
            _editor->onLeftClick(input.getViewRelativeMousePos());
        }

        auto view = winMgr.getMainRenderTexture().getView().clone();
        view->setSize({1024.f, 1024.f});
        // winMgr.getView().zoom(_editorZoom);

        const auto xOff =
            ((float)input.checkPressed(hg::in::PK_D) - (float)input.checkPressed(hg::in::PK_A)) * 8.f;
        const auto yOff =
            ((float)input.checkPressed(hg::in::PK_S) - (float)input.checkPressed(hg::in::PK_W)) * 8.f;
        view->setCenter(view->getCenter() + hg::math::Vector2f{xOff, yOff});
    }

    void _eventDraw1() override {
        HG_HARD_ASSERT(_editor != nullptr);

        auto& winMgr = ccomp<spe::WindowManagerInterface>();
        _editor->draw(winMgr);
    }

    void _eventDrawGUI() override {
        HG_HARD_ASSERT(_editor != nullptr);

        auto& winMgr = ccomp<spe::WindowManagerInterface>();
        _editor->drawGui(winMgr);
    }

private:
    Editor* _editor = nullptr;

    float _editorZoom = 1.f;
};

#define PRIORITY_EDITOR_DRIVER  1
#define PRIORITY_WINDOW_MANAGER 0

#ifdef _MSC_VER
#define TICK_RATE  60
#define FRAME_RATE 120
#else
#define TICK_RATE  60
#define FRAME_RATE 60
#endif

std::unique_ptr<spe::GameContext> CreateContex() {
    auto uwgaSystem = uwga::CreateGraphicsSystem("SFML");

    spe::GameContext::RuntimeConfig rtConfig{spe::TickRate{TICK_RATE}};
    auto                            ctx = std::make_unique<spe::GameContext>(rtConfig);

    // Add a WindowManager
    auto winMgr = hg::QAO_Create<spe::DefaultWindowManager>(ctx->getQAORuntime().nonOwning(),
                                                            PRIORITY_WINDOW_MANAGER);
    // clang-format off
    spe::WindowManagerInterface::WindowConfig windowConfig{
        .size  = {1200, 800},
        .title = "Antimony Animator",
        .style = hg::uwga::WindowStyle::DEFAULT
    };
    spe::WindowManagerInterface::MainRenderTextureConfig mrtConfig{
        .size   = {800, 600},
        .smooth = true
    };
    spe::WindowManagerInterface::TimingConfig timingConfig{
    #ifdef _MSC_VER
        spe::FrameRate{FRAME_RATE},
        spe::PREVENT_BUSY_WAIT_ON,
        spe::VSYNC_OFF
    #else
        FRAME_RATE,
        spe::PREVENT_BUSY_WAIT_OFF,
        spe::VSYNC_OFF
    #endif
    };
    // clang-format on
    winMgr->setToNormalMode(uwgaSystem, windowConfig, mrtConfig, timingConfig);
    winMgr->setMainRenderTextureDrawPosition(spe::WindowManagerInterface::DrawPosition::FIT);
    winMgr->setStopIfCloseClicked(true);

    ctx->attachAndOwnComponent(std::move(winMgr));

    return ctx;
}

/*
 * antimony.exe dir/to/sprites sprite_base_name master_cnt lerp_cnt
 */
int main(int argc, char* argv[]) try {
    hg::log::SetMinimalLogSeverity(hg::log::Severity::Info);

    if (argc != 5) {
        HG_LOG_INFO(LOG_ID,
                    "Usage:\n    antimony.exe dir/to/sprites sprite_base_name master_cnt lerp_cnt");
        return EXIT_SUCCESS;
    }
    const std::filesystem::path spriteDir      = argv[1];
    const std::string           spriteBaseName = argv[2];
    const hg::PZInteger         masterCount    = std::stoi(argv[3]);
    const hg::PZInteger         lerpCount      = std::stoi(argv[3]);

    auto context = CreateContex();

    MasterLoader loader{context->getComponent<spe::WindowManagerInterface>().getGraphicsSystem(),
                        spriteDir,
                        spriteBaseName,
                        masterCount};

    Editor editor{loader, masterCount, lerpCount};

    auto editorDriver = hg::QAO_Create<EditorDriver>(context->getQAORuntime(), PRIORITY_EDITOR_DRIVER);
    editorDriver->init(editor);

    const auto status = context->runFor(-1);

    return status;
} catch (const hg::TracedException& ex) {
    HG_LOG_FATAL(LOG_ID, "Traced exception caught: {}", ex.getFormattedDescription());
    return EXIT_FAILURE;
} catch (const std::exception& ex) {
    HG_LOG_FATAL(LOG_ID, "Exception caught: {}", ex.what());
    return EXIT_FAILURE;
}
