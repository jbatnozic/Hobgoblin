# Copyright 2026 Jovan Batnozic. Released under MS-PL licence in Serbia.
# See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

project("Cinnabar1")

add_executable(${PROJECT_NAME}
    "Source/InteriorWorld/Interior_world.cpp"

    "Source/Asteroid.cpp"
    "Source/Attachable_ghost.cpp"
    "Source/Main.cpp"
    "Source/Overworld_manager_default.cpp"
    "Source/Game_context_factory.cpp"
    "Source/Graph_of_attachables.cpp"
    "Source/Graphics_system_provider.cpp"
    "Source/Main_game_flow_manager_default.cpp"
    "Source/Poly_shape.cpp"
    "Source/Ship_attachable.cpp"
    "Source/Ship_controller.cpp"
    "Source/Ship_starting_block.cpp"
    "Source/Interactivity_manager_default.cpp"
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
