// Copyright 2026 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#include <Interactivity_manager.hpp>

#include <Hobgoblin/HGExcept.hpp>

#include <QAOMessages/Handle_pncs_event.hpp>

namespace cinnabar {

#define MODE_EDGE spe::WindowFrameInputView::Mode::Edge

InteractivityManager::InteractivityManager(QAO_InstGuard aInstGuard)
    : spe::NonstateObject{aInstGuard,
                          QAO_ExeCon::GAMEPLAY,
                          PRIORITY_INTERACTIVITYMGR,
                          QAO_STATIC_NAME("cinnabar::InteractivityManager")} {}

void InteractivityManager::pushClickableObject(QAO_GenericId aClickableId, std::intptr_t aUserData) {
    HG_ASSERT(getRuntime()->getCurrentEvent() == QAO_Event::BEGIN_UPDATE);
    _clickablesStack.push_back({aClickableId, aUserData});
}

void InteractivityManager::_didAttach(QAO_Runtime& aRuntime) {
    spe::NonstateObject::_didAttach(aRuntime);
    _winMgr = &ccomp<MWindow>();
}

void InteractivityManager::_eventPreUpdate() {
    _clickablesStack.clear();
}

void InteractivityManager::_eventBeginUpdate() {
    while (!_clickablesStack.empty()) {
        auto& info = _clickablesStack.back();

        auto instance = getRuntime()->find(info.id);
        if (instance.isNull()) {
            _clickablesStack.pop_back();
            continue;
        }

        const auto input = _winMgr->getInput();

        PNCSEventData evData = {.mbLeftDown  = input.checkPressed(hg::in::MB_LEFT, MODE_EDGE),
                                .mbRightDown = input.checkPressed(hg::in::MB_RIGHT, MODE_EDGE),
                                .mbLeftUp    = input.checkReleased(hg::in::MB_LEFT, MODE_EDGE),
                                .mbRightUp   = input.checkReleased(hg::in::MB_RIGHT, MODE_EDGE),
                                .userData    = info.userData};

        (void)QAO_SendMessage<HandlePNCSEvent>(*instance, &evData);

        break; // End the loop after the "foreground-most" object receives its message
    }
}

void InteractivityManager::_eventPreDraw() {
    while (!_clickablesStack.empty()) {
        auto& info = _clickablesStack.back();

        auto instance = getRuntime()->find(info.id);
        if (instance.isNull()) {
            _clickablesStack.pop_back();
            continue;
        }

        PNCSEventData evData = {.mbLeftDown  = false,
                                .mbRightDown = false,
                                .mbLeftUp    = false,
                                .mbRightUp   = false,
                                .userData    = info.userData};

        (void)QAO_SendMessage<HandlePNCSEvent>(*instance, &evData);

        break; // End the loop after the "foreground-most" object receives its message
    }
}

} // namespace cinnabar
