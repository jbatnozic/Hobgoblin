// Copyright 2026 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#pragma once

#include <Engine.hpp>

#include <Interactivity_manager_interface.hpp>

#include <vector>

namespace cinnabar {

class InteractivityManager
    : public spe::NonstateObject
    , public InteractivityManagerInterface {
public:
    InteractivityManager(QAO_InstGuard aInstGuard);

    void pushClickableObject(QAO_GenericId aClickableId, std::intptr_t aUserData) override;

private:
    MWindow* _winMgr = nullptr;

    struct ClickableInfo {
        QAO_GenericId id;
        std::intptr_t userData;
    };

    std::vector<ClickableInfo> _clickablesStack;

    void _didAttach(QAO_Runtime& aRuntime) override;

    void _eventPreUpdate() override;
    void _eventBeginUpdate() override;
    void _eventPreDraw() override;
};

QAO_REGISTER_CLASS(InteractivityManager, cinnabar_InteractivityManager) {
    QAO_LOCAL_ALIAS(C, klass);
    klass.setSuperclass<spe::NonstateObject>();
}

} // namespace cinnabar
