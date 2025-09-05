// Copyright 2024 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#define HOBGOBLIN_SHORT_NAMESPACE
#include <Hobgoblin/Common.hpp>
#include <Hobgoblin/Format.hpp>
#include <Hobgoblin/HGExcept.hpp>
#include <Hobgoblin/Input.hpp>
#include <Hobgoblin/Logging.hpp>
#include <Hobgoblin/UWGA.hpp>
#include <Hobgoblin/Unicode.hpp>

#include <SPeMPE/SPeMPE.hpp>

#include <cstdlib>
#include <memory>
#include <string>

namespace uwga = hg::uwga;
namespace spe  = ::jbatnozic::spempe;
using namespace hg::in;

constexpr auto LOG_ID = "FTT";

#ifdef _MSC_VER
#define TICK_RATE  60
#define FRAME_RATE 120
#else
#define TICK_RATE  60
#define FRAME_RATE 60
#endif

void SimpleTest() {
    auto uwgaSystem = uwga::CreateGraphicsSystem("SFML");
    auto window = uwgaSystem->createRenderWindow(800, 800, uwga::WindowStyle::DEFAULT, "FTT (Simple)");
    auto view   = window->createDefaultView();

    view->setCenter({0.f, 0.f});
    window->setView(*view);
    window->setFramerateLimit(TICK_RATE);

    uwga::CircleShape circle{*uwgaSystem, 32.f};
    circle.setOrigin({32.f, 32.f});
    circle.setFillColor(uwga::COLOR_RED);

    while (true) {
        uwga::WindowEvent ev;
        while (window && window->pollEvent(ev)) {
            ev.visit([&](const uwga::WindowEvent::Closed&) {
                window.reset();
            });
        }
        if (!window) {
            break;
        }

        const auto lr         = (float)CheckPressedPK(PK_D) - (float)CheckPressedPK(PK_A);
        const auto ud         = (float)CheckPressedPK(PK_S) - (float)CheckPressedPK(PK_W);
        const auto multiplier = 8.f;
        view->setCenter(view->getCenter() + hg::math::Vector2f{-lr * multiplier, -ud * multiplier});
        window->setView(*view);

        window->clear();
        window->draw(circle);
        window->display();
    }
}

#define PRIORITY_EDITOR_DRIVER  1
#define PRIORITY_WINDOW_MANAGER 0

std::unique_ptr<spe::GameContext> CreateContex() {
    auto uwgaSystem = uwga::CreateGraphicsSystem("SFML");

    spe::GameContext::RuntimeConfig rtConfig{spe::TickRate{TICK_RATE}};
    auto                            ctx = std::make_unique<spe::GameContext>(rtConfig);

    // Add a WindowManager
    auto winMgr = hg::QAO_Create<spe::DefaultWindowManager>(ctx->getQAORuntime().nonOwning(),
                                                            PRIORITY_WINDOW_MANAGER);
    // clang-format off
    spe::WindowManagerInterface::WindowConfig windowConfig{
        .size = {800, 800},
        .title = "FTT (SPeMPE)",
        .style = hg::uwga::WindowStyle::DEFAULT
    };
    spe::WindowManagerInterface::MainRenderTextureConfig mrtConfig{
        .size = {1024, 1024},
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

    auto view = winMgr->getActiveCanvas().getView().clone();
    view->setCenter({0.f, 0.f});
    winMgr->getActiveCanvas().setView(*view);

    ctx->attachAndOwnComponent(std::move(winMgr));

    return ctx;
}

class Driver : public spe::NonstateObject {
public:
    Driver(hg::QAO_InstGuard aInstGuard, int aExecutionPriority)
        : spe::NonstateObject{aInstGuard, SPEMPE_TYPEID_SELF, aExecutionPriority, "Driver"} {}

    void _eventUpdate1() override {
        auto&       winMgr = ccomp<spe::WindowManagerInterface>();
        const auto& input  = winMgr.getInput();

        const auto lr         = (float)input.checkPressed(PK_D) - (float)input.checkPressed(PK_A);
        const auto ud         = (float)input.checkPressed(PK_S) - (float)input.checkPressed(PK_W);
        const auto multiplier = 8.f;

        auto view = winMgr.getActiveCanvas().getView().clone();
        view->setCenter(view->getCenter() + hg::math::Vector2f{-lr * multiplier, -ud * multiplier});
        winMgr.getActiveCanvas().setView(*view);
    }

    void _eventDraw1() override {
        auto& winMgr = ccomp<spe::WindowManagerInterface>();
        auto& canvas = winMgr.getActiveCanvas();

        uwga::CircleShape circle{canvas.getSystem(), 32.f};
        circle.setOrigin({32.f, 32.f});
        circle.setFillColor(uwga::COLOR_RED);
        canvas.draw(circle);
    }
};

void TestWithSPeMPE() {
    auto context = CreateContex();

    auto driver = hg::QAO_Create<Driver>(context->getQAORuntime(), PRIORITY_EDITOR_DRIVER);

    context->runFor(-1);
}

int main(int argc, char* argv[]) {
    SimpleTest();
    TestWithSPeMPE();
}
