// Copyright 2024 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#ifndef SPEMPE_UTILITY_RPC_RECEIVER_CONTEXT_USER_HPP
#define SPEMPE_UTILITY_RPC_RECEIVER_CONTEXT_USER_HPP

#include <SPeMPE/Managers/Networking_manager.hpp>
#include <SPeMPE/Utility/Rpc_receiver_context_template.hpp>

namespace jbatnozic {
namespace spempe {

using RPCReceiverContext = RPCReceiverContextTemplate<NetworkingManager>;

inline void* MapSyncIdToObject(const RPCReceiverContext& aRpcReceiverContext, SyncId aSyncId) {
    return aRpcReceiverContext.netwMgr.mapSyncIdToObject(aSyncId);
}

} // namespace spempe
} // namespace jbatnozic

#endif // !SPEMPE_UTILITY_RPC_RECEIVER_CONTEXT_USER_HPP

