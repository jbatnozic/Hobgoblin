// Copyright 2026 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#pragma once

#include <Hobgoblin/Common.hpp>
#include <Hobgoblin/HGExcept.hpp>
#include <Hobgoblin/Logging.hpp>
#include <Hobgoblin/QAO.hpp>
#include <Hobgoblin/RigelNet.hpp>
#include <SPeMPE/SPeMPE.hpp>

namespace cinnabar {

namespace hg   = ::jbatnozic::hobgoblin;
namespace spe  = ::jbatnozic::spempe;
namespace uwga = ::jbatnozic::hobgoblin::uwga;
using namespace hg::qao; // All names from QAO are prefixed with QAO_
using namespace hg::rn;  // All names from RigelNet are prefixed with RN_

using MInput        = spe::InputSyncManagerInterface;
using MLobbyBackend = spe::LobbyBackendManagerInterface;
using MNetworking   = spe::NetworkingManagerInterface;
using MWindow       = spe::WindowManagerInterface;

constexpr const char* LOG_ID = "Connabar1";

// #define PRIORITY_VARMAPMGR     16
// #define PRIORITY_NETWORKMGR    15
// #define PRIORITY_LOBBYBACKMGR  14
// #define PRIORITY_LOBBYFRONTMGR 13
// #define PRIORITY_AUTHMGR       12
// #define PRIORITY_GAMEPLAYMGR   10
// #define PRIORITY_INPUTMGR       7
// #define PRIORITY_PLAYERAVATAR   5
#define PRIORITY_WINDOWMGR      0

#define STATE_BUFFERING_LENGTH 2

} // namespace cinnabar
