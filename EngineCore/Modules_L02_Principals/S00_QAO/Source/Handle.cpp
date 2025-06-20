// Copyright 2025 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#include <Hobgoblin/HGExcept.hpp>
#include <Hobgoblin/QAO/Base.hpp>
#include <Hobgoblin/QAO/Handle.hpp>
#include <Hobgoblin/QAO/Runtime.hpp>

#include <Hobgoblin/Private/Pmacro_define.hpp>

HOBGOBLIN_NAMESPACE_BEGIN
namespace qao {
namespace qao_detail {

void DetachFromRuntime(QAO_Base& aObject) {
    auto* rt = aObject.getRuntime();
    if (rt != nullptr) {
        auto handle = hobgoblin::MoveToUnderlying(rt->detachObject(aObject.getId()));
        HG_HARD_ASSERT(!handle.isOwning());
    }
}

} // namespace qao_detail
} // namespace qao
HOBGOBLIN_NAMESPACE_END

#include <Hobgoblin/Private/Pmacro_undef.hpp>
