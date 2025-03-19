// Copyright 2024 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#include <SPeMPE/SPeMPE.hpp>

#include <memory>

namespace jbatnozic {
namespace gridgoblin {
namespace editor {

namespace spe = ::jbatnozic::spempe;

#define TICK_RATE  60
#define FRAME_RATE 60

#define WINDOW_WIDTH  1200
#define WINDOW_HEIGHT 900

// Main Render Texture (MRT) size determines the resolution
// at which the game will be rendered internally.
#define MRT_WIDTH  960
#define MRT_HEIGHT 540

std::unique_ptr<spe::GameContext> CreateSPeMPEContext() {
    auto context =
        std::make_unique<spe::GameContext>(spe::GameContext::RuntimeConfig{spe::TickRate{TICK_RATE}});
    context->setToMode(spe::GameContext::Mode::Client);

    // Window manager
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
            "DjoxTrauma",
            hg::win::WindowStyle::Default
        },
        spe::WindowManagerInterface::MainRenderTextureConfig{{MRT_WIDTH, MRT_HEIGHT}},
        timingConfig);
    // clang-format on
    winMgr->setMainRenderTextureDrawPosition(spe::WindowManagerInterface::DrawPosition::Fit);
    winMgr->setStopIfCloseClicked(true);

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
    std::filesystem::path root = std::filesystem::current_path();
    for (int i = 0; i < 10; i += 1) {
        if (std::filesystem::exists(root / "Assets")) {
            break;
        }
        root = root.parent_path();
    }
    for (const FontFace& face : font_faces) {
        Rml::LoadFontFace((root / "Assets/fonts/").string() + face.filename, face.fallback_face);
    }

#ifndef NDEBUG
    Rml::Debugger::Initialise(&(winMgr->getGUIContext()));
    Rml::Debugger::SetVisible(true);
#endif

    context->attachAndOwnComponent(std::move(winMgr));

    return context;
}

} // namespace editor
} // namespace gridgoblin
} // namespace jbatnozic
