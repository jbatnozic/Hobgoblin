// Copyright 2024 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#include "Context_factory.hpp"
#include "Priorities.hpp"
#include "Editor_ui_driver.hpp"
#include "Editor_world_driver.hpp"
#include "Resource_holder_default.hpp"

#include <Hobgoblin/Common/Build_type.hpp>
#include <SPeMPE/SPeMPE.hpp>

#include <memory>

namespace jbatnozic {
namespace gridgoblin {
namespace editor {

namespace hg  = ::jbatnozic::hobgoblin;
namespace spe = ::jbatnozic::spempe;

#define TICK_RATE  60
#define FRAME_RATE 60

#define WINDOW_WIDTH  1200
#define WINDOW_HEIGHT 900

// Main Render Texture (MRT) size determines the resolution
// at which the game will be rendered internally.
#define MRT_WIDTH  1920
#define MRT_HEIGHT 1080

std::unique_ptr<spe::GameContext> CreateEditorSPeMPEContext(const EditorConfig& aConfig) {
    auto context =
        std::make_unique<spe::GameContext>(spe::GameContext::RuntimeConfig{spe::TickRate{TICK_RATE}});
    context->setToMode(spe::GameContext::Mode::Client);

    // Instantiate Window manager
    auto winMgr = QAO_UPCreate<spe::DefaultWindowManager>(context->getQAORuntime().nonOwning(),
                                                          PRIORITY_WINDOWMGR);
    spe::WindowManagerInterface::TimingConfig timingConfig{
#if defined(_MSC_VER)
        spe::FrameRate{FRAME_RATE},
        spe::PREVENT_BUSY_WAIT_ON,
        spe::VSYNC_ON
#elif defined(__linux__)
        spe::FrameRate{FRAME_RATE},
        spe::PREVENT_BUSY_WAIT_ON,
        spe::VSYNC_OFF
#elif defined(__APPLE__)
        FRAME_RATE * 2,
        spe::PREVENT_BUSY_WAIT_OFF,
        spe::VSYNC_OFF
#else // ?
#error "Unknown target OS configuration"
#endif
    };

    // clang-format off
    winMgr->setToNormalMode(
        spe::WindowManagerInterface::WindowConfig{
            hg::win::VideoMode{WINDOW_WIDTH, WINDOW_HEIGHT},
            "GridGoblin Editor",
            hg::win::WindowStyle::Default
        },
        spe::WindowManagerInterface::MainRenderTextureConfig{{MRT_WIDTH, MRT_HEIGHT}},
        timingConfig);
    // clang-format on
    winMgr->setMainRenderTextureDrawPosition(spe::WindowManagerInterface::DrawPosition::Fit);
    winMgr->setStopIfCloseClicked(true);

#if HG_BUILD_TYPE == HG_DEBUG
    Rml::Debugger::Initialise(&(winMgr->getGUIContext()));
    Rml::Debugger::SetVisible(true);
#endif

    context->attachAndOwnComponent(std::move(winMgr));

    // Load fonts (global) - goes after the WindowManager because it initializes the RmlUi system
    struct FontFace {
        Rml::String filename;
        bool        fallback_face;
    };
    FontFace font_faces[] = {
        {   "LatoLatin-Regular.ttf", false},
        {    "LatoLatin-Italic.ttf", false},
        {      "LatoLatin-Bold.ttf", false},
        {"LatoLatin-BoldItalic.ttf", false},
    };
    for (const FontFace& face : font_faces) {
        Rml::LoadFontFace((aConfig.assetsDir / "Fonts/").string() + face.filename, face.fallback_face);
    }

    // Instantiate Resource holder
    {
        auto resHolder =
            std::make_unique<DefaultResourceHolder>(aConfig.definitionsDir, aConfig.spritesDir);
        context->attachAndOwnComponent(std::move(resHolder));
    }

    // Instantiate World driver
    {
        using namespace hg::qao;
        auto* driver = QAO_PCreate<EditorWorldDriver>(context->getQAORuntime(), PRIORITY_WORLDDRIVER);
        driver->init(aConfig);
    }

    // Instantiate UI driver
    {
        using namespace hg::qao;
        auto* driver = QAO_PCreate<EditorUiDriver>(context->getQAORuntime(), PRIORITY_UIDRIVER);
        driver->init(aConfig.assetsDir, aConfig.definitionsDir);
    }

    return context;
}

} // namespace editor
} // namespace gridgoblin
} // namespace jbatnozic
