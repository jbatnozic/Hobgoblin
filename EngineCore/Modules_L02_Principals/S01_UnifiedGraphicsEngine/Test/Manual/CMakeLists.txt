# Copyright 2024 Jovan Batnozic. Released under MS-PL licence in Serbia.
# See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

project("UnifiedGraphicsEngine.ManualTest")

add_executable(${PROJECT_NAME}
    "Empty_renderwindow_test.cpp"
    "UGE_manual_test.cpp"
)

target_link_libraries(${PROJECT_NAME}
PUBLIC
    # Foundation
    "Hobgoblin.HGExcept"
    "Hobgoblin.ManualTestFramework"

    # Principals
    "Hobgoblin.UnifiedGraphicsEngine"
)
