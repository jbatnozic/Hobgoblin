// Copyright 2026 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#pragma once

#include <CBR/Managers/UserInterface/Interactivity_manager.hpp>
#include <Engine.hpp>

#include <memory>
#include <optional>
#include <vector>

namespace cinnabar {

/*!
 * \brief Default implementation of `AuxiliaryGameplayManager`.
 *
 * Dependencies:
 * - `LobbyBackendManager`
 * - `InputSyncManager`
 * - `WindowManager`
 *
 * Event listing:
 * - _eventPreUpdate()   => Clean PNCS (Point n' Click System) data.
 * - _eventBeginUpdate() => (Client only) Send (upload) local controls, dispatch PNCS events;
 *                          (Host only) Gater remote player controls;
 *                          (Dev only) Dispatch PNCS events.
 * - _eventPreDraw()     => Dispatch mouseover/mouse hover PNCS events.
 *
 * Execution priority hint:
 * - As late as possible; after all interactible entities (to gather PNCS registrations);
 *   ideally just before `WindowManager`.
 */
class DefaultInteractivityManager
    : public spe::NonstateObject
    , public InteractivityManager {
public:
    DefaultInteractivityManager(QAO_InstGuard aInstGuard);

    ~DefaultInteractivityManager();

    Mode getMode() const override;

    void setToClientMode() override;
    void setToHostMode(hg::PZInteger aPlayerCount) override;
    void setToDevMode() override;

    PlayerViewFocus getCurrentPlayerViewFocus() const override;

    void pushClickableObject(QAO_GenericId                           aClickableId,
                             int                                     aFinegrainedPriority,
                             std::intptr_t                           aUserData,
                             std::function<bool(hg::math::Vector2d)> aQuickMouseOverCheck,
                             std::function<bool(hg::math::Vector2d)> aFullMouseOverCheck) override;

    const PlayerControls& getControlsOfPlayer(hg::PZInteger aPlayerIndex) const override;

    const PlayerControls& getControlsOfClient(int aClientIndex) const override;

private:
    Mode            _mode            = Mode::INITIAL;
    PlayerViewFocus _playerViewFocus = PlayerViewFocus::OVERWORLD; // TODO: temporary! should be DEFAULT!

    MWindow*       _winMgr          = nullptr;
    MLobbyBackend* _lobbyBackendMgr = nullptr;

    std::unique_ptr<hg::uwga::RenderTexture> _secondaryFocusRenderTexture;

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

    class HostData;
    std::unique_ptr<HostData> _hostData;

    void _didAttach(QAO_Runtime& aRuntime) override;

    void _eventPreUpdate() override;
    void _eventBeginUpdate() override;
    void _eventPreDraw() override;

    void _initSecondaryFocusRenderTexture();

    hg::math::Vector2d _getMouseWorldPosition();

    void _gatherRemotePlayerControls();
    void _sendLocalPlayerControls();
    void _dispatchPNCSEvents();

    hg::uwga::Canvas& _getActiveCanvasForShipInterior() override;
    hg::uwga::Canvas& _getActiveCanvasForOverworld() override;
};

QAO_REGISTER_CLASS(DefaultInteractivityManager, cinnabar_InteractivityManager) {
    QAO_LOCAL_ALIAS(C, klass);
    klass.setSuperclass<spe::NonstateObject>();
}

} // namespace cinnabar
