# Copyright 2026 Jovan Batnozic. Released under MS-PL licence in Serbia.
# See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

project("Cinnabar1")

add_executable(${PROJECT_NAME}
    "Source/CBR/GameContext/Game_context_factory.cpp"
    "Source/CBR/GameContext/Graphics_system_provider.cpp"

    "Source/CBR/InteriorWorld/Interior_world.cpp"

    "Source/CBR/Managers/Gameplay/Main_game_flow_manager_default.cpp"
    "Source/CBR/Managers/Gameplay/Overworld_manager_default.cpp"
    "Source/CBR/Managers/UserInterface/Interactivity_manager_default.cpp"
    "Source/CBR/Managers/UserInterface/Lobby_frontend_manager_default.cpp"

    "Source/CBR/Overworld/Asteroid.cpp"

    "Source/CBR/Ship/Attachable_ghost.cpp"
    "Source/CBR/Ship/Graph_of_attachables.cpp"
    "Source/CBR/Ship/Ship_attachable.cpp"
    "Source/CBR/Ship/Ship_controller.cpp"
    "Source/CBR/Ship/Ship_starting_block.cpp"

    "Source/CBR/Utility/Poly_shape.cpp"

    "Source/Main.cpp"
)

target_link_libraries(${PROJECT_NAME}
PUBLIC
    # Foundation
    "Hobgoblin.Common"
    "Hobgoblin.Format"
    "Hobgoblin.Preprocessor"
    "Hobgoblin.HGExcept"
    "Hobgoblin.Logging"
    "Hobgoblin.Unicode"

    # Utilities
    "Hobgoblin.Math"
    "Hobgoblin.Utility"

    # Principals
    "Hobgoblin.ChipmunkPhysics"
    "Hobgoblin.Input"
    "Hobgoblin.QAO"
    "Hobgoblin.RigelNet"
    "Hobgoblin.Alvin"
    "Hobgoblin.UWGA"
    "Hobgoblin.RmlUi"

    # Overlays
    "GridGoblin"
    "SPeMPE"

    # External
    # TODO
)

target_include_directories(${PROJECT_NAME} PRIVATE "Source/")

# Copy the assets (RmlUi documents, fonts, sprites, ...) next to the executable.
# https://stackoverflow.com/questions/48468205/cmake-forcing-copy-of-specified-files-in-post-build-even-if-no-change-in-source
add_custom_command(
    OUTPUT "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/assets"
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_CURRENT_SOURCE_DIR}/assets"
            "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/assets"
    DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/assets"
)

add_custom_target("cinnabar1-copy-assets" DEPENDS "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/assets")

add_dependencies(${PROJECT_NAME} "cinnabar1-copy-assets")
