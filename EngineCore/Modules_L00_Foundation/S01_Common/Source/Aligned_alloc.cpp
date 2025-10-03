// Copyright 2025 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#include <Hobgoblin/Common/Aligned_alloc.hpp>

#ifndef _MSC_VER
#include <cstdlib>
#else
#include <malloc.h>
#endif

#include <Hobgoblin/Private/Pmacro_define.hpp>

HOBGOBLIN_NAMESPACE_BEGIN

void* AlignedAlloc(std::size_t aAlignment, std::size_t aSize) {
    if (aSize == 0 || (aSize % aAlignment != 0)) {
        return nullptr;
    }

#ifndef _MSC_VER
    return std::aligned_alloc(aAlignment, aSize);
#else
    return _aligned_malloc(aSize, aAlignment);
#endif
}

void AlignedFree(void* aPtr) {
#ifndef _MSC_VER
    std::free(aPtr);
#else
    _aligned_free(aPtr);
#endif
}

HOBGOBLIN_NAMESPACE_END

#include <Hobgoblin/Private/Pmacro_undef.hpp>
