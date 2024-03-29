
#include <SPeMPE/GameObjectFramework/Sync_details.hpp>
#include <SPeMPE/GameObjectFramework/Synchronized_object_registry.hpp>
#include <SPeMPE/Managers/Networking_manager_interface.hpp>
#include <SPeMPE/Utility/Rpc_receiver_context_template.hpp>

#include <Hobgoblin/RigelNet.hpp>
#include <Hobgoblin/RigelNet_macros.hpp>

#include <algorithm>

namespace jbatnozic {
namespace spempe {

namespace {
RN_DEFINE_RPC(USPEMPE_DeactivateObject, RN_ARGS(SyncId, aSyncId)) {
    RN_NODE_IN_HANDLER().callIfClient(
        [=](hg::RN_ClientInterface& client) {
        const auto rc    = SPEMPE_GET_RPC_RECEIVER_CONTEXT(client);
        auto  regId      = rc.netwMgr.getRegistryId();
        auto& syncObjReg = *reinterpret_cast<detail::SynchronizedObjectRegistry*>(regId.address);

        syncObjReg.deactivateObject(aSyncId, rc.pessimisticLatencyInSteps);
    });

    RN_NODE_IN_HANDLER().callIfServer(
        [](hg::RN_ServerInterface&) {
        throw hg::RN_IllegalMessage("Server received a sync message");
    });
}
} // namespace

//
// SyncFlags
//

SyncFlags operator|(SyncFlags aLhs, SyncFlags aRhs) {
    return static_cast<SyncFlags>(
        static_cast<detail::SyncFlagsUnderlyingType>(aLhs) | 
        static_cast<detail::SyncFlagsUnderlyingType>(aRhs)
    );
}

SyncFlags& operator|=(SyncFlags& aLhs, SyncFlags aRhs) {
    return aLhs = static_cast<SyncFlags>(
        static_cast<detail::SyncFlagsUnderlyingType>(aLhs) | 
        static_cast<detail::SyncFlagsUnderlyingType>(aRhs)
     );
}

SyncFlags operator&(SyncFlags aLhs, SyncFlags aRhs) {
    return static_cast<SyncFlags>(
        static_cast<detail::SyncFlagsUnderlyingType>(aLhs) &
        static_cast<detail::SyncFlagsUnderlyingType>(aRhs)
     );
}

SyncFlags& operator&=(SyncFlags& aLhs, SyncFlags aRhs) {
    return aLhs = static_cast<SyncFlags>(
        static_cast<detail::SyncFlagsUnderlyingType>(aLhs) &
        static_cast<detail::SyncFlagsUnderlyingType>(aRhs)
     );
}

bool HasPacemakerPulse(SyncFlags aFlags) {
    return ((aFlags & SyncFlags::PacemakerPulse) != SyncFlags::None);
}

bool HasFullState(SyncFlags aFlags) {
    return ((aFlags & SyncFlags::FullState) != SyncFlags::None);
}

hg::util::Packet& operator<<(hg::util::PacketExtender& aPacket, SyncFlags aFlags) {
    return (aPacket << static_cast<detail::SyncFlagsUnderlyingType>(aFlags));
}

hg::util::Packet& operator>>(hg::util::PacketExtender& aPacket, SyncFlags& aFlags) {
    const auto value = aPacket->extractNoThrow<detail::SyncFlagsUnderlyingType>();
    aFlags = static_cast<SyncFlags>(value);
    return *aPacket;
}

//
// SyncDetails
//

SyncDetails::SyncDetails(detail::SynchronizedObjectRegistry& aRegistry)
    : _registry{&aRegistry}
{
}

hg::RN_NodeInterface& SyncDetails::getNode() const {
    return _registry->getNode();
}

const std::vector<hg::PZInteger>& SyncDetails::getRecepients() const {
    return _recepients;
}

SyncFlags SyncDetails::getFlags() const {
    return _flags;
}

void SyncDetails::filterSyncs(const SyncDetails::FilterPrecidateFunc& aPredicate) {
    constexpr static bool REMOVE_IF_DO_REMOVE   = true;
    constexpr static bool REMOVE_IF_DONT_REMOVE = false;

    auto& reg = *_registry;
    const SyncId forObject = _forObject;

    auto iter = std::remove_if(_recepients.begin(), _recepients.end(),
        [&](hg::PZInteger aRecepient) -> bool {
        switch (aPredicate(aRecepient)) {
        case FilterResult::FullSync:
            return REMOVE_IF_DONT_REMOVE;

        case FilterResult::Skip:
            return REMOVE_IF_DO_REMOVE;

        case FilterResult::Deactivate:
            if (!reg.isObjectDeactivatedForClient(forObject, aRecepient)) {
                reg.setObjectDeactivatedFlagForClient(forObject, aRecepient, true);
                Compose_USPEMPE_DeactivateObject(reg.getNode(), aRecepient, forObject);
            }
            return REMOVE_IF_DO_REMOVE;

        default:
            assert(false && "Unreachable");
            break;
        }
    });

    _recepients.erase(iter, _recepients.end());
}

} // namespace spempe
} // namespace jbatnozic
