# Copyright 2024 Jovan Batnozic. Released under MS-PL licence in Serbia.
# See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

project("Hobgoblin.UWGA.ManualTest")

add_executable(${PROJECT_NAME}
    "Empty_renderwindow_test.cpp"
    "Rendertexture_in_renderwindow_test.cpp"
    "Sprite_loader_test.cpp"
    "Square_in_renderwindow_test.cpp"
    "Transform_in_renderwindow_test.cpp"
    "Textured_vertices_in_renderwindow_test.cpp"

    "UWGA_manual_test.cpp"
)

target_link_libraries(${PROJECT_NAME}
PUBLIC
    # Foundation
    "Hobgoblin.HGExcept"
    "Hobgoblin.ManualTestFramework"

    # Principals
    "Hobgoblin.UWGA"
)

hg_copy_test_assets(${PROJECT_NAME} "Assets")
