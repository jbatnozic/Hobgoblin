// Copyright 2026 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#pragma once

#include <CBR/Managers/Gameplay/Main_game_flow_manager.hpp>
#include <Engine.hpp>

#include <cstdint>

namespace cinnabar {

/*!
 * \brief Default implementation of `MainGameFlowManager`.
 *
 * Dependencies:
 * - <none>
 *
 * Event listing:
 * - _eventDisplay() => Adjust global EXECON level.
 *
 * Execution priority hint:
 * - Literally last, even after `WindowManager`.
 */
class DefaultMainGameFlowManager
    : public spe::NonstateObject
    , public MainGameFlowManager
    , private spe::NetworkingEventListener {
public:
    DefaultMainGameFlowManager(QAO_InstGuard aInstGuard);

private:
    void _didAttach(QAO_Runtime& aRuntime) override;
    void _willDetach(QAO_Runtime& aRuntime) override;

    void _eventDisplay() override;

    void onNetworkingEvent(const RN_Event& aEvent) override;

    std::uint32_t _iterOrdinal = 0;
};

QAO_REGISTER_CLASS(DefaultMainGameFlowManager, cinnabar_DefaultMainGameFlowManager) {
    QAO_LOCAL_ALIAS(C, klass);
    klass.setSuperclass<spe::NonstateObject>();
}

} // namespace cinnabar
