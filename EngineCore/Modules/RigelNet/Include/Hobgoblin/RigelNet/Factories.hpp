#ifndef UHOBGOBLIN_RN_FACTORIES_HPP
#define UHOBGOBLIN_RN_FACTORIES_HPP

#include <Hobgoblin/RigelNet/Configuration.hpp>
#include <Hobgoblin/RigelNet/Client_interface.hpp>
#include <Hobgoblin/RigelNet/Server_interface.hpp>

#include <memory>
#include <string>

#include <Hobgoblin/Private/Pmacro_define.hpp>

HOBGOBLIN_NAMESPACE_BEGIN
namespace rn {

#define DEFAULT_MAX_PACKET_SIZE 512
#define DEFAULT_STACK RN_NetworkingStack::Default

class RN_ServerFactory {
public:
    static std::unique_ptr<RN_ServerInterface> createServer(
        RN_Protocol aProtocol,
        std::string aPassphrase,
        PZInteger aServerSize = 1,
        PZInteger aMaxPacketSize = DEFAULT_MAX_PACKET_SIZE,
        RN_NetworkingStack aNetworkingStack = DEFAULT_STACK);

    //! Returns an object that fulfills the interface but doesn't do anything.
    static std::unique_ptr<RN_ServerInterface> createDummyServer();
};

class RN_ClientFactory {
public:
    static std::unique_ptr<RN_ClientInterface> createClient(
        RN_Protocol aProtocol,
        std::string aPassphrase,
        PZInteger aMaxPacketSize = DEFAULT_MAX_PACKET_SIZE,
        RN_NetworkingStack aNetworkingStack = DEFAULT_STACK);

    //! Returns an object that fulfills the interface but doesn't do anything.
    static std::unique_ptr<RN_ClientInterface> createDummyClient();
};

#undef DEFAULT_STACK
#undef DEFAULT_MAX_PACKET_SIZE

} // namespace rn
HOBGOBLIN_NAMESPACE_END

#include <Hobgoblin/Private/Pmacro_undef.hpp>
#include <Hobgoblin/Private/Short_namespace.hpp>

#endif // !UHOBGOBLIN_RN_FACTORIES_HPP