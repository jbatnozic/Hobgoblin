// Copyright 2025 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#ifndef UHOBGOBLIN_COMMON_ALIGNED_ALLOC_HPP
#define UHOBGOBLIN_COMMON_ALIGNED_ALLOC_HPP

#include <cstddef>

#include <Hobgoblin/Private/Pmacro_define.hpp>

HOBGOBLIN_NAMESPACE_BEGIN

//! Allocate `aSize` bytes of uninitialized storage whose alignment is specified by `aAlignment`
//!
//! \param aAlignment - specifies the alignment. Must be a valid alignment supported by the
//!                     implementation (fundamental alignments are always supported).
//! \param aSize      - number of bytes to allocate. An integral multiple of `aAlignment`.
//!
//! \returns pointer to the allocated memory region. If the allocation fails, or `aSize` is zero or
//!          not an integral multiple of `aAlignment`, returns `nullptr`.
//!
//! \warning if a non-null pointer is returned, it must later be freed using `AlignedFree` to avoid
//!          memory leaks.
void* AlignedAlloc(std::size_t aAlignment, std::size_t aSize);

//! Free the memory allocated using `AlignedAlloc`. Does nothing if `aPtr` is null.
void AlignedFree(void* aPtr);

HOBGOBLIN_NAMESPACE_END

#include <Hobgoblin/Private/Pmacro_undef.hpp>
#include <Hobgoblin/Private/Short_namespace.hpp>

#endif // !UHOBGOBLIN_COMMON_ALIGNED_ALLOC_HPP
