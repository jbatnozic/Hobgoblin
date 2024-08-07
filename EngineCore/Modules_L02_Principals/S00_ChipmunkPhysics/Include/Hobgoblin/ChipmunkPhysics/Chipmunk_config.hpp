// Copyright 2024 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#ifndef UHOBGOBLIN_CHIPMUNKPHYSICS_CHIPMUNK_CONFIG_HPP
#define UHOBGOBLIN_CHIPMUNKPHYSICS_CHIPMUNK_CONFIG_HPP

#include <Hobgoblin/Utility/Any_ptr.hpp>

#include <cstdint>

#define CP_BOOL_TYPE bool
#define cpTrue       true
#define cpFalse      false

#define CP_BITMASK_TYPE      ::std::uintptr_t
#define CP_DATA_POINTER_TYPE ::jbatnozic::hobgoblin::util::AnyPtr

#endif // !UHOBGOBLIN_CHIPMUNKPHYSICS_CHIPMUNK_CONFIG_HPP
