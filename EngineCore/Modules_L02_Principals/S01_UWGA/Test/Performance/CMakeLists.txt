# Copyright 2025 Jovan Batnozic. Released under MS-PL licence in Serbia.
# See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

project("Hobgoblin.UWGA.PerformanceTest")

add_executable(${PROJECT_NAME}
    "Sprite_drawing_performance_test.cpp"
    "UWGA_performance_test.cpp"
)

target_link_libraries(${PROJECT_NAME}
PUBLIC
  # Common
  "Hobgoblin.ManualTestFramework"

  # Principals
  "Hobgoblin.UWGA"

  # External
  "sfml::sfml"
)

hg_copy_test_assets(${PROJECT_NAME} "Assets")
