// Copyright 2026 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#pragma once

#include <Engine.hpp>

namespace cinnabar {

struct SyncRecepientInfo {
    hg::PZInteger  clientIndex;
    spe::SyncFlags syncFlags;
};

QAO_DEFINE_MESSAGE(SyncCreateAndUpdateToClient, const SyncRecepientInfo*);

} // namespace cinnabar
