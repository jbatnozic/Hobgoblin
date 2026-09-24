// Copyright 2026 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#pragma once

#include <Engine.hpp>

#include <string>

namespace cinnabar {

//! Presents the lobby (backed by a `spe::LobbyBackendManager`) to the local player
//! and lets an authorized player control it.
class LobbyFrontendManager : public spe::ContextComponent {
public:
    enum class Mode {
        Uninitialized,
        HeadlessHost,
        Client,
    };

    ~LobbyFrontendManager() override = default;

    virtual void setToHeadlessHostMode()                                                 = 0;
    virtual void setToClientMode(const std::string& aName, const std::string& aUniqueId) = 0;

    virtual Mode getMode() const = 0;

private:
    SPEMPE_CTXCOMP_TAG("cinnabar::LobbyFrontendManager");
};

using MLobbyFrontend = LobbyFrontendManager;

} // namespace cinnabar
