// Copyright 2026 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#pragma once

#include "Stream_io_overloads.hpp" // IWYU pragma: keep

#include <Hobgoblin/Common.hpp>
#include <Hobgoblin/HGExcept.hpp>
#include <Hobgoblin/Logging.hpp>
#include <Hobgoblin/QAO.hpp>
#include <Hobgoblin/RigelNet.hpp>
#include <Hobgoblin/UWGA/Render_window.hpp>
#include <SPeMPE/SPeMPE.hpp>

namespace cinnabar {

namespace hg   = ::jbatnozic::hobgoblin;
namespace spe  = ::jbatnozic::spempe;
namespace uwga = ::jbatnozic::hobgoblin::uwga;
using namespace hg::qao; // All names from QAO are prefixed with QAO_
using namespace hg::rn;  // All names from RigelNet are prefixed with RN_

using MInput        = spe::InputSyncManager;
using MLobbyBackend = spe::LobbyBackendManager;
using MNetworking   = spe::NetworkingManager;
using MWindow       = spe::WindowManager;

constexpr const char* LOG_ID = "Cinnabar1";

// PRIORITIES BEGIN ///////////////////////////////////////////////////////////

// First-block managers/controllers
#define PRIORITY_VARMAPMGR     2600
#define PRIORITY_NETWORKMGR    2500
#define PRIORITY_INPUTMGR      2400
#define PRIORITY_LOBBYBACKMGR  2300
#define PRIORITY_LOBBYEVENTMGR 2200
#define PRIORITY_LOBBYFRONTMGR 2100
#define PRIORITY_AUTHMGR       2000

// Gameplay entities & gameplay managers/controllers
#define PRIORITY_AUXGAMEPLAYMGR 1200
#define PRIORITY_ENTITIES       1100
#define PRIORITY_ENVIRONMENTMGR 1000

// Last-block managers/controllers
#define PRIORITY_INTERACTIVITYMGR  300
#define PRIORITY_NETWORKSENDHELPER 200
#define PRIORITY_WINDOWMGR         100
#define PRIORITY_MAINGAMEFLOWMGR   000

// PRIORITIES END /////////////////////////////////////////////////////////////

#define STATE_BUFFERING_LENGTH 2

// clang-format off
#define BEFRIEND_QAO_CREATE             \
    template <class T, class... taArgs> \
    friend auto ::jbatnozic::hobgoblin::qao::QAO_Create(QAO_RuntimeRef, taArgs&&...) -> QAO_Handle<T>

#define BEFRIEND_QAO_CREATE_FOR_FACTORIES                                              \
    template <class T, class... taArgs>                                                \
    friend auto ::jbatnozic::hobgoblin::qao::QAO_Create_USE_IN_FACTORY_FUNCTIONS_ONLY( \
        QAO_RuntimeRef,                                                                \
        taArgs&&...                                                                    \
    ) -> QAO_Handle<T>
// clang-format on

} // namespace cinnabar
