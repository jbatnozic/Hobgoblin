// Copyright 2026 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#pragma once

#include <Engine.hpp>

#include <memory>

namespace cinnabar {

class GraphicsSystemProvider : public spe::ContextComponent {
public:
    SPEMPE_CTXCOMP_TAG("cinnabar::GraphicsSystemProvider");

    GraphicsSystemProvider(std::shared_ptr<uwga::System> aUwgaSystem);
    
    uwga::System& getSystem() const;

    std::shared_ptr<uwga::System> getSystemPtr() const;

private:
    std::shared_ptr<uwga::System> _system;
};

} // namespace cinnabar
