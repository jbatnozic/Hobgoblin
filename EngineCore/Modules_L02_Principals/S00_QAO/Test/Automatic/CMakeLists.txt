# Copyright 2025 Jovan Batnozic. Released under MS-PL licence in Serbia.
# See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

project("Hobgoblin.QAO.AutomaticTest")

find_package(GTest CONFIG REQUIRED)

add_executable(${PROJECT_NAME}
    "Priority_resolver_test.cpp"
    "Test_with_runtime.cpp"
)

target_link_libraries(${PROJECT_NAME}
    PUBLIC
  
    # Foundation
    "Hobgoblin_L00_S01_Common"
  
    # Principals
    "Hobgoblin_L02_S00_QAO"
  
    # External
    "gtest::gtest"
)
