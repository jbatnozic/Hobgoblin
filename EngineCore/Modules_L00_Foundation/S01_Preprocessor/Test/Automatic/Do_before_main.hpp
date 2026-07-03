// Copyright 2026 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#include <Hobgoblin/Preprocessor.hpp>

inline int& GetTestCounter() {
    static int c = 0;
    return c;
}

HG_DO_BEFORE_MAIN(IncrementTestCounter) {
    int& counter = GetTestCounter();
    ++counter;
}