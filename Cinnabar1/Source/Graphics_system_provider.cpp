// Copyright 2026 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#include <Graphics_system_provider.hpp>

namespace cinnabar {

GraphicsSystemProvider::GraphicsSystemProvider(std::shared_ptr<uwga::System> aUwgaSystem)
    : _system{std::move(aUwgaSystem)} {}

uwga::System& GraphicsSystemProvider::getSystem() const {
    return *_system;
}

std::shared_ptr<uwga::System> GraphicsSystemProvider::getSystemPtr() const {
    return _system;
}

} // namespace cinnabar
