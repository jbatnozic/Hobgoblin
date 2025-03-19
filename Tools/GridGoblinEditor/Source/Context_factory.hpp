// Copyright 2024 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#include <SPeMPE/SPeMPE.hpp>

#include <memory>

namespace jbatnozic {
namespace gridgoblin {
namespace editor {

namespace spe = ::jbatnozic::spempe;

std::unique_ptr<spe::GameContext> CreateSPeMPEContext();

} // namespace editor
} // namespace gridgoblin
} // namespace jbatnozic
