// Copyright 2026 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#pragma once

#include <Engine.hpp>

namespace cinnabar {

class InteriorWorld;

//! An extended version of `QAO_Runtime` (intended as a member of `InteriorWorld`),
//! to be used to own and run objects internal to an `InteriorWorld`.
class InternalRuntime : public QAO_Runtime {
public:
    InternalRuntime(InteriorWorld&    aInteriorWorld,
                    hg::util::AnyPtr  aUserData,
                    const QAO_ExeCon* aExeconAddress)
        : QAO_Runtime{aUserData, aExeconAddress}
        , _parentInteriorWorld{aInteriorWorld} {}

    InteriorWorld& getInteriorWorld() const {
        return _parentInteriorWorld;
    }

private:
    InteriorWorld& _parentInteriorWorld;
};

} // namespace cinnabar
