// Copyright 2026 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#include <Interactivity_manager_default.hpp>

#include <Hobgoblin/HGExcept.hpp>

#include <QAOMessages/Handle_pncs_event.hpp>

#include <algorithm>

namespace cinnabar {

#define MODE_EDGE spe::WindowFrameInputView::Mode::Edge

DefaultInteractivityManager::DefaultInteractivityManager(QAO_InstGuard aInstGuard)
    : spe::NonstateObject{aInstGuard,
                          QAO_ExeCon::GAMEPLAY,
                          PRIORITY_INTERACTIVITYMGR,
                          QAO_STATIC_NAME("cinnabar::DefaultInteractivityManager")} {}

void DefaultInteractivityManager::pushClickableObject(
    QAO_GenericId                           aClickableId,
    int                                     aFinegrainedPriority,
    std::intptr_t                           aUserData,
    std::function<bool(hg::math::Vector2d)> aQuickMouseOverCheck,
    std::function<bool(hg::math::Vector2d)> aFullMouseOverCheck) //
{
    HG_ASSERT(getRuntime()->getCurrentEvent() == QAO_Event::BEGIN_UPDATE);

    if (!aQuickMouseOverCheck(_getMouseWorldPosition())) {
        return; // The object doesn't intersect with the mouse cursor (but no guarantee that it does!)
    }

    _clickables.push_back({.id                  = aClickableId,
                           .finegrainedPriority = aFinegrainedPriority,
                           .userData            = aUserData,
                           .fullMouseOverCheck  = std::move(aFullMouseOverCheck)});
}

void DefaultInteractivityManager::_didAttach(QAO_Runtime& aRuntime) {
    spe::NonstateObject::_didAttach(aRuntime);
    _winMgr = &ccomp<MWindow>();
}

void DefaultInteractivityManager::_eventPreUpdate() {
    _clickables.clear();
    _mouseWorldPos.reset();
}

void DefaultInteractivityManager::_eventBeginUpdate() {
    std::stable_sort(_clickables.begin(), _clickables.end());

    while (!_clickables.empty()) {
        auto& info = _clickables.back();

        auto instance = getRuntime()->find(info.id);
        if (instance.isNull()) {
            _clickables.pop_back();
            continue;
        }

        if (!info.fullMouseOverCheck(_getMouseWorldPosition())) {
            _clickables.pop_back(); // The object doesn't intersect with the mouse cursor
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

void DefaultInteractivityManager::_eventPreDraw() {
    // Here all the clickables are already sorted

    while (!_clickables.empty()) {
        auto& info = _clickables.back();

        auto instance = getRuntime()->find(info.id);
        if (instance.isNull()) {
            _clickables.pop_back();
            continue;
        }

        if (!info.fullMouseOverCheck(_getMouseWorldPosition())) {
            _clickables.pop_back(); // The object doesn't intersect with the mouse cursor
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

hg::math::Vector2d DefaultInteractivityManager::_getMouseWorldPosition() {
    if (!_mouseWorldPos.has_value()) {
        _mouseWorldPos = _winMgr->getInput().getViewRelativeMousePos();
    }

    return *_mouseWorldPos;
}

} // namespace cinnabar
