// Copyright 2024 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#ifndef UHOBGOBLIN_RN_UDP_SERVER_HPP
#define UHOBGOBLIN_RN_UDP_SERVER_HPP

#include <Hobgoblin/RigelNet/Remote_info.hpp>
#include <Hobgoblin/RigelNet/Server_interface.hpp>
#include <Hobgoblin/RigelNet/Telemetry.hpp>
#include <Hobgoblin/Utility/No_copy_no_move.hpp>

#include "Node_base.hpp"
#include "Socket_adapter.hpp"
#include "Udp_connector_impl.hpp"

#include <chrono>
#include <memory>
#include <string>
#include <vector>

#include <Hobgoblin/Private/Pmacro_define.hpp>

HOBGOBLIN_NAMESPACE_BEGIN
namespace rn {

class RN_UdpServerImpl
    : public RN_NodeBase
    , public RN_ServerInterface {
public:
    //! Create the server with the specified size but don't start it.
    RN_UdpServerImpl(std::string        passphrase,
                     PZInteger          size,
                     RN_NetworkingStack networkingStack,
                     PZInteger          aMaxPacketSize);

    ~RN_UdpServerImpl();

    ///////////////////////////////////////////////////////////////////////////
    // SERVER CONTROL                                                        //
    ///////////////////////////////////////////////////////////////////////////

    void start(std::uint16_t localPort) override;

    void stop(bool aNotifyClients = true) override;

    void resize(PZInteger newSize) override;

    void setTimeoutLimit(std::chrono::microseconds limit) override;

    void setRetransmitPredicate(RN_RetransmitPredicate pred) override;

    // From RN_NodeInterface:

    RN_Telemetry update(RN_UpdateMode mode) override;

    void addEventListener(NeverNull<RN_EventListener*> aEventListener) override;

    void removeEventListener(NeverNull<RN_EventListener*> aEventListener) override;

    ///////////////////////////////////////////////////////////////////////////
    // CLIENT MANAGEMENT                                                     //
    ///////////////////////////////////////////////////////////////////////////

    const RN_ConnectorInterface& getClientConnector(PZInteger clientIndex) const override;

    RN_ConnectorInterface& getClientConnector(PZInteger clientIndex) override;

    void kickClient(PZInteger          aClientIndex,
                    bool               aNotifyRemote = true,
                    const std::string& aMessage      = "") override;

    ///////////////////////////////////////////////////////////////////////////
    // STATE INSPECTION                                                      //
    ///////////////////////////////////////////////////////////////////////////

    bool isRunning() const override;

    PZInteger getSize() const override;

    const std::string& getPassphrase() const override;

    std::chrono::microseconds getTimeoutLimit() const override;

    std::uint16_t getLocalPort() const override;

    int getSenderIndex() const override;

    // From RN_NodeInterface:

    bool isServer() const noexcept override;

    RN_Protocol getProtocol() const noexcept override;

    RN_NetworkingStack getNetworkingStack() const noexcept override;

    ///////////////////////////////////////////////////////////////////////////
    // NOT INHERITED                                                         //
    ///////////////////////////////////////////////////////////////////////////

    //! Returns client index (or -1 on failure)
    int acceptLocalConnection(RN_UdpConnectorImpl& localPeer, const std::string& passphrase);

private:
    RN_SocketAdapter _socket;
    PZInteger        _maxPacketSize;

    std::vector<std::unique_ptr<RN_UdpConnectorImpl>> _clients;

    std::string               _passphrase;
    std::chrono::microseconds _timeoutLimit = std::chrono::microseconds{0};
    RN_RetransmitPredicate    _retransmitPredicate;
    int                       _senderIndex = -1;
    bool                      _running     = false;

    util::Packet* _currentPacket = nullptr;

    RN_Telemetry _updateReceive();
    RN_Telemetry _updateSend();
    int          _findConnector(sf::IpAddress addr, std::uint16_t port) const;
    void         _handlePacketFromUnknownSender(sf::IpAddress senderIp,
                                                std::uint16_t senderPort,
                                                util::Packet& packet);

    void _compose(RN_ComposeForAllType receiver, const void* data, std::size_t sizeInBytes) override;
    void _compose(PZInteger receiver, const void* data, std::size_t sizeInBytes) override;
    util::Packet* _getCurrentPacket() override;
    void          _setUserData(util::AnyPtr userData) override;
    util::AnyPtr  _getUserData() const override;
};

} // namespace rn
HOBGOBLIN_NAMESPACE_END

#include <Hobgoblin/Private/Pmacro_undef.hpp>
#include <Hobgoblin/Private/Short_namespace.hpp>

#endif // !UHOBGOBLIN_RN_UDP_SERVER_HPP
