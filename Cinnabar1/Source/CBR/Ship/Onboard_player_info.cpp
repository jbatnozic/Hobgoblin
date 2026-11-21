// Copyright 2026 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#include "Onboard_player_info.hpp"

namespace cinnabar {

// MARK: Mappings

void OnboardPlayerInfo::updateMappings(const spe::LobbyBackendManager& aLobbyBackendManager) {
    const auto size = hg::pztos(aLobbyBackendManager.getSize());
    _clientToPlayerMapping.resize(size);
    _playerToClientMapping.resize(size);
    for (std::size_t i = 0; i < size; ++i) {
        _clientToPlayerMapping[i] = aLobbyBackendManager.clientIdxToPlayerIdx((int)i);
        _playerToClientMapping[i] = aLobbyBackendManager.playerIdxToClientIdx((int)i);
    }
}

std::int8_t OnboardPlayerInfo::playerIdxToClientIdx(std::int8_t aPlayerIndex) const {
    HG_ASSERT(aPlayerIndex >= 0);
    if ((std::size_t)aPlayerIndex >= _playerToClientMapping.size()) {
        return spe::CLIENT_INDEX_UNKNOWN;
    }
    return _playerToClientMapping[aPlayerIndex];
}

std::int8_t OnboardPlayerInfo::clientIdxToPlayerIdx(std::int8_t aClientIndex) const {
    HG_ASSERT(aClientIndex >= 0);
    if ((std::size_t)aClientIndex >= _clientToPlayerMapping.size()) {
        return spe::PLAYER_INDEX_UNKNOWN;
    }
    return _clientToPlayerMapping[aClientIndex];
}

// MARK: Player management

bool OnboardPlayerInfo::addOnboardPlayer(std::int8_t aPlayerIndex) {
    HG_ASSERT(aPlayerIndex >= 0);
    if (!_bitset.getBit(aPlayerIndex * BITS_PER_PC + 0)) {
        _bitset.setBit(aPlayerIndex * BITS_PER_PC + 0);

        if (const auto clientIndex = playerIdxToClientIdx(aPlayerIndex); !(clientIndex < 0)) {
            setSecondaryVisibleStateSyncedForClient(clientIndex, false);
        }

        return true;
    }
    return false;
}

bool OnboardPlayerInfo::removeOnboardPlayer(std::int8_t aPlayerIndex) {
    HG_ASSERT(aPlayerIndex >= 0);
    if (_bitset.getBit(aPlayerIndex * BITS_PER_PC + 0)) {
        _bitset.clearBit(aPlayerIndex * BITS_PER_PC + 0);
        return true;
    }
    return false;
}

bool OnboardPlayerInfo::isPlayerOnboard(std::int8_t aPlayerIndex) const {
    HG_ASSERT(aPlayerIndex >= 0);
    return _bitset.getBit(aPlayerIndex * BITS_PER_PC + 0);
}

// MARK: SVS Management

void OnboardPlayerInfo::setSecondaryVisibleStateSyncedForClient(std::int8_t aClientIndex, bool aSynced) {
    if (aSynced) {
        _bitset.setBit(aClientIndex * BITS_PER_PC + 1);
    } else {
        _bitset.clearBit(aClientIndex * BITS_PER_PC + 1);
    }
}

bool OnboardPlayerInfo::isSecondaryVisibleStateSyncedForClient(std::int8_t aClientIndex) const {
    return _bitset.getBit(aClientIndex * BITS_PER_PC + 1);
}

// MARK: Client Management

bool OnboardPlayerInfo::isClientOnboard(std::int8_t aClientIndex) const {
    const auto playerIndex = clientIdxToPlayerIdx(aClientIndex);
    if (playerIndex < 0) {
        // This situation can happen if a player is connected but hasn't been assigned
        // to a slot yet (lobby hasn't been locked in yet).
        return false;
    }
    return isPlayerOnboard(playerIndex);
}

spe::SyncFilterStatus OnboardPlayerInfo::getSyncFilterStatusForClient(std::int8_t aClientIndex) const {
    return spe::SyncFilterStatus::REGULAR_SYNC; // TODO
}

} // namespace cinnabar
