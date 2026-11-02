// Copyright 2024 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#include <Hobgoblin/RigelNet/Node_interface.hpp>

#include <Hobgoblin/RigelNet/Client_interface.hpp>
#include <Hobgoblin/RigelNet/Server_interface.hpp>

#include <cstring>

#include <Hobgoblin/Private/Pmacro_define.hpp>

HOBGOBLIN_NAMESPACE_BEGIN
namespace rn {

RN_NodeInterface::~RN_NodeInterface() = default;

void RN_NodeInterface::setUserData(std::nullptr_t) {
    _setUserData(nullptr);
}

void RN_NodeInterface::callIfClient(std::function<void(RN_ClientInterface& client)> func) {
    if (!isServer()) {
        func(static_cast<RN_ClientInterface&>(SELF));
    }
}

void RN_NodeInterface::callIfServer(std::function<void(RN_ServerInterface& client)> func) {
    if (isServer()) {
        func(static_cast<RN_ServerInterface&>(SELF));
    }
}

void RN_NodeInterface::illegalIfClient(std::string_view aMessageName) {
    if (!isServer()) {
        char buffer[256];
        snprintf(buffer,
                 256,
                 "Client received illegal message intended only for the Server: '%s'",
                 aMessageName.data());
        throw RN_IllegalMessage{buffer};
    }
}

void RN_NodeInterface::illegalIfServer(std::string_view aMessageName) {
    if (isServer()) {
        char buffer[256];
        snprintf(buffer,
                 256,
                 "Server received illegal message intended only for Clients: '%s'",
                 aMessageName.data());
        throw RN_IllegalMessage{buffer};
    }
}

} // namespace rn
HOBGOBLIN_NAMESPACE_END

#include <Hobgoblin/Private/Pmacro_undef.hpp>
