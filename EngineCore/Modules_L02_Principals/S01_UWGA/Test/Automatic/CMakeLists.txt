# Copyright 2025 Jovan Batnozic. Released under MS-PL licence in Serbia.
# See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

project("Hobgoblin.UWGA.AutomaticTest")

find_package(GTest CONFIG REQUIRED)

add_executable(${PROJECT_NAME}
    "Draw_batching_decorator_fnl_test.cpp"
    "Sprite_loader_test.cpp"
)

target_link_libraries(${PROJECT_NAME}
PUBLIC
  "Hobgoblin.Common"
  "Hobgoblin.Unicode"
  "Hobgoblin.UWGA"

  # External
  "gtest::gtest"
  "sfml::sfml"
)

hg_copy_test_assets(${PROJECT_NAME} "Assets")

include(GoogleTest)
gtest_discover_tests("Hobgoblin.UWGA.AutomaticTest")
