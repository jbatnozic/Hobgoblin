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
    HG_ASSERT(aPlayerIndex >= 0 && (std::size_t)aPlayerIndex < _playerToClientMapping.size());
    return _playerToClientMapping[aPlayerIndex];
}

std::int8_t OnboardPlayerInfo::clientIdxToPlayerIdx(std::int8_t aClientIndex) const {
    HG_ASSERT(aClientIndex >= 0 && (std::size_t)aClientIndex < _clientToPlayerMapping.size());
    return _clientToPlayerMapping[aClientIndex];
}

// MARK: Player management

bool OnboardPlayerInfo::addOnboardPlayer(std::int8_t aPlayerIndex) {
    HG_ASSERT(aPlayerIndex >= 0 && (std::size_t)aPlayerIndex < _playerToClientMapping.size());
    const auto clientIndex = _playerToClientMapping[aPlayerIndex];
    if (!_clientStatusBits.getBit(clientIndex * BITS_PER_CLIENT + 0)) {
        _clientStatusBits.setBit(clientIndex * BITS_PER_CLIENT + 0);
        return true;
    }
    return false;
}

bool OnboardPlayerInfo::removeOnboardPlayer(std::int8_t aPlayerIndex) {
    HG_ASSERT(aPlayerIndex >= 0 && (std::size_t)aPlayerIndex < _playerToClientMapping.size());
    const auto clientIndex = _playerToClientMapping[aPlayerIndex];
    if (_clientStatusBits.getBit(clientIndex * BITS_PER_CLIENT + 0)) {
        _clientStatusBits.clearBit(clientIndex * BITS_PER_CLIENT + 0);
        return true;
    }
    return false;
}

bool OnboardPlayerInfo::isPlayerOnboard(std::int8_t aPlayerIndex) const {
    HG_ASSERT(aPlayerIndex >= 0 && (std::size_t)aPlayerIndex < _playerToClientMapping.size());
    const auto clientIndex = _playerToClientMapping[aPlayerIndex];
    return _clientStatusBits.getBit(clientIndex * BITS_PER_CLIENT + 0);
}

// MARK: SVS Management

void OnboardPlayerInfo::setSecondaryVisibleStateSyncedForClient(std::int8_t aClientIndex, bool aSynced) {
    if (aSynced) {
        _clientStatusBits.setBit(aClientIndex * BITS_PER_CLIENT + 1);
    } else {
        _clientStatusBits.clearBit(aClientIndex * BITS_PER_CLIENT + 1);
    }
}

bool OnboardPlayerInfo::isSecondaryVisibleStateSyncedForClient(std::int8_t aClientIndex) const {
    return _clientStatusBits.getBit(aClientIndex * BITS_PER_CLIENT + 1);
}

// MARK: Client Management

bool OnboardPlayerInfo::isClientOnboard(std::int8_t aClientIndex) const {
    return _clientStatusBits.getBit(aClientIndex * BITS_PER_CLIENT + 0);
}

spe::SyncFilterStatus OnboardPlayerInfo::getSyncFilterStatusForClient(std::int8_t aClientIndex) const {
    return spe::SyncFilterStatus::REGULAR_SYNC; // TODO
}

} // namespace cinnabar
