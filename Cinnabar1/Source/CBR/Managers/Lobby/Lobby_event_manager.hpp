// Copyright 2026 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#pragma once

#include <Hobgoblin/Common/Nullability.hpp>
#include <Engine.hpp>

namespace cinnabar {

class LobbyEventListener {
public:
    virtual ~LobbyEventListener() = default;

    virtual void onLobbyEvent(const spe::LobbyBackendEvent& aEvent) = 0;
};

class LobbyEventManager : public spe::ContextComponent {
public:
    virtual void addEventListener(hg::NeverNull<LobbyEventListener*> aListener) = 0;
    virtual void removeEventListener(hg::NeverNull<LobbyEventListener*> aListener) = 0;

private:
    SPEMPE_CTXCOMP_TAG("cinnabar::LobbyEventManager");
};

using MLobbyEvent = LobbyEventManager;

} // namespace cinnabar
