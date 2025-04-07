# Copyright 2024 Jovan Batnozic. Released under MS-PL licence in Serbia.
# See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

project("GridGoblin.Editor")

find_package(RapidJSON CONFIG REQUIRED)

add_executable(${PROJECT_NAME}
    "Source/Definitions.cpp"
    "Source/Context_factory.cpp"
    "Source/Editor_ui_driver.cpp"
    "Source/Editor_world_driver.cpp"
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
    "Hobgoblin.Window"
    "Hobgoblin.Graphics"

    # Overlays
    "GridGoblin"
    "SPeMPE"

    # External
    "rapidjson"
)

# TODO: Chunk conversions to and from json strings could be even faster if std::string is not used
#       (not a big concern for now; it's a possible micro-optimization)
target_compile_definitions(${PROJECT_NAME}
PRIVATE
    "RAPIDJSON_HAS_STDSTRING=1"
)

hg_copy_test_assets(${PROJECT_NAME} "Assets")
