// Copyright 2026 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#pragma once

#include <Engine.hpp>

#include <Hobgoblin/Utility/Dynamic_bitset.hpp>
#include <Hobgoblin/Utility/Compressed_small_vector.hpp>

namespace cinnabar {

//! \brief Holds information about players/clients onboard a `ShipController`.
class OnboardPlayerInfo {
public:
    ///////////////////////////////////////////////////////////////////////////
    // MARK: MAPPINGS                                                        //
    ///////////////////////////////////////////////////////////////////////////

    void updateMappings(const spe::LobbyBackendManager& aLobbyBackendManager);

    std::int8_t playerIdxToClientIdx(std::int8_t aPlayerIndex) const;

    std::int8_t clientIdxToPlayerIdx(std::int8_t aClientIndex) const;

    ///////////////////////////////////////////////////////////////////////////
    // MARK: PLAYER MANAGEMENT                                               //
    ///////////////////////////////////////////////////////////////////////////

    //! \returns whether the player was indeed added (true) or he was already present (false).
    //! \note when this function returns true, the associated `ShipController` should sync its
    //!       complete internal state to this player.
    bool addOnboardPlayer(std::int8_t aPlayerIndex);

    //! \returns whether the player was indeed removed (true) or he was already absent (false).
    //! \note when this function returns true, the associated `ShipController` should instruct
    //!       this player's client to drop the whole internal state.
    bool removeOnboardPlayer(std::int8_t aPlayerIndex);

    bool isPlayerOnboard(std::int8_t aPlayerIndex) const;

    ///////////////////////////////////////////////////////////////////////////
    // MARK: SVS MANAGEMENT                                                  //
    ///////////////////////////////////////////////////////////////////////////

    void setSecondaryVisibleStateSyncedForClient(std::int8_t aClientIndex, bool aSynced);

    bool isSecondaryVisibleStateSyncedForClient(std::int8_t aClientIndex) const;

    ///////////////////////////////////////////////////////////////////////////
    // MARK: CLIENT MANAGEMENT                                               //
    ///////////////////////////////////////////////////////////////////////////

    bool isClientOnboard(std::int8_t aClientIndex) const;

    spe::SyncFilterStatus getSyncFilterStatusForClient(std::int8_t aClientIndex) const;

private:
    static constexpr hg::PZInteger BITS_PER_CLIENT = 2;

    //! This bitset stores information about clients in the following way:
    //! - there are 2 bits per client, so bits 0 and 1 pertain to client 0, bits 2 and 3 pertain to
    //!   client 1, and so on.
    //! - of the two bits pertaining to an individual client, the less significant bit (e.g., bit 0 for
    //!   client 0) tells us whether the client is "onboard" the ship [controller] associated with this
    //!   `OnboardPlayerInfo` object and whether SVS data *should* be synced to this client.
    //! - of the two bits pertaining to an individual client, the more significant bit (e.g., bit 1 for
    //!   client 0) tells us whether SVS data has been synced to this client thus far. 
    hg::util::DynamicBitset _clientStatusBits;

    //! Maps player indices to client indices.
    //! `_playerToClientMapping[N]` == client index of player N.
    hg::util::CompressedSmallVector<std::int8_t, 15> _playerToClientMapping;

    //! Maps client indices to player indices.
    //! `_clientToPlayerMapping[N]` == player index of client N.
    hg::util::CompressedSmallVector<std::int8_t, 15> _clientToPlayerMapping;


};

} // namespace cinnabar
