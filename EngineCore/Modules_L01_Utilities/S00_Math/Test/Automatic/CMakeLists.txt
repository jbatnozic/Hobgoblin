# Copyright 2024 Jovan Batnozic. Released under MS-PL licence in Serbia.
# See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

project("Hobgoblin.Math.AutomaticTest")

find_package(GTest CONFIG REQUIRED)

add_executable(${PROJECT_NAME}
    "Vector_test.cpp"
)

target_link_libraries(${PROJECT_NAME}
PUBLIC
    # Utilities
    "Hobgoblin.Utility"

    # External
    "gtest::gtest"
)

include(GoogleTest)
gtest_discover_tests(${PROJECT_NAME})
