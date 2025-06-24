// Copyright 2024 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#ifndef SPEMPE_GAME_OBJECT_FRAMEWORK_SYNC_ID_HPP
#define SPEMPE_GAME_OBJECT_FRAMEWORK_SYNC_ID_HPP

#include <cstdint>

namespace jbatnozic {
namespace spempe {

//! Number identifying a SynchronizedObject.
using SyncId = std::uint64_t;

//! When instantiating a new `SynchronizedObject`, pass it this constant as a constructor
//! parameter to indicate that a new synchronization ID should be assigned to it by the
//! `NetworkingManager` when it is attached to the runtime/context.
constexpr SyncId SYNC_ID_NEW = 0;

} // namespace spempe
} // namespace jbatnozic

#endif // !SPEMPE_GAME_OBJECT_FRAMEWORK_SYNC_ID_HPP
