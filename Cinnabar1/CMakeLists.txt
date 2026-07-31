# Copyright 2026 Jovan Batnozic. Released under MS-PL licence in Serbia.
# See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

project("Cinnabar1")

add_executable(${PROJECT_NAME}
    "Source/Main.cpp"
    "Source/Game_context_factory.cpp"
    "Source/Main_game_flow_manager.cpp"
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
