# Copyright 2026 Jovan Batnozic. Released under MS-PL licence in Serbia.
# See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

project("Hobgoblin.Preprocessor.AutomaticTest")

find_package(GTest CONFIG REQUIRED)

add_executable(${PROJECT_NAME}
    "Do_before_main_1.cpp"
    "Do_before_main_2.cpp"
    "Preprocessor_test.cpp"
)

target_link_libraries(${PROJECT_NAME}
PUBLIC
    # Foundation
    "Hobgoblin.Preprocessor"

    # External
    "gtest::gtest"
)

include(GoogleTest)
gtest_discover_tests("Hobgoblin.Preprocessor.AutomaticTest")
