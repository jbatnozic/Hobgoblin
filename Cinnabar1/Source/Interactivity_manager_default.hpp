// Copyright 2026 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#pragma once

#include <Engine.hpp>

#include <Interactivity_manager.hpp>

#include <optional>
#include <vector>

namespace cinnabar {

class DefaultInteractivityManager
    : public spe::NonstateObject
    , public InteractivityManager {
public:
    DefaultInteractivityManager(QAO_InstGuard aInstGuard);

    void pushClickableObject(QAO_GenericId                           aClickableId,
                             int                                     aFinegrainedPriority,
                             std::intptr_t                           aUserData,
                             std::function<bool(hg::math::Vector2d)> aQuickMouseOverCheck,
                             std::function<bool(hg::math::Vector2d)> aFullMouseOverCheck) override;

private:
    MWindow* _winMgr = nullptr;

    struct ClickableInfo {
        QAO_GenericId                           id;
        int                                     finegrainedPriority;
        std::intptr_t                           userData;
        std::function<bool(hg::math::Vector2d)> fullMouseOverCheck;

        bool operator<(const ClickableInfo& aOther) const {
            // the inverted > operator is intentional!
            // objects with higher priorities must be pushed towards the start of the vector
            // (they are more obscured by other objects)
            return finegrainedPriority > aOther.finegrainedPriority;
        }
    };

    std::vector<ClickableInfo> _clickables;

    std::optional<hg::math::Vector2d> _mouseWorldPos;

    void _didAttach(QAO_Runtime& aRuntime) override;

    void _eventPreUpdate() override;
    void _eventBeginUpdate() override;
    void _eventPreDraw() override;

    hg::math::Vector2d _getMouseWorldPosition();
};

QAO_REGISTER_CLASS(DefaultInteractivityManager, cinnabar_InteractivityManager) {
    QAO_LOCAL_ALIAS(C, klass);
    klass.setSuperclass<spe::NonstateObject>();
}

} // namespace cinnabar
