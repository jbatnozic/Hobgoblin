// Copyright 2024 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#pragma once

#include <SPeMPE/SPeMPE.hpp>

#include <filesystem>
#include <memory>

namespace jbatnozic {
namespace gridgoblin {
namespace editor {

namespace hg  = ::jbatnozic::hobgoblin;
namespace spe = ::jbatnozic::spempe;

class EditorUiDriver : public spe::NonstateObject {
public:
    explicit EditorUiDriver(hg::QAO_RuntimeRef aRuntimeRef, int aExecutionPriority);

    ~EditorUiDriver();

    void init(const std::filesystem::path& aAssetsDir, const std::filesystem::path& aDefinitionsDir);

private:
    class Impl;
    std::unique_ptr<Impl> _impl;

    void _eventBeginUpdate() override;
    void _eventUpdate1() override;
    // void _eventPostUpdate() override;
    void _eventDrawGUI() override;
};

} // namespace editor
} // namespace gridgoblin
} // namespace jbatnozic