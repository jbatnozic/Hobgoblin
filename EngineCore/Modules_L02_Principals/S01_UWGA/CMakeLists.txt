# Copyright 2024 Jovan Batnozic. Released under MS-PL licence in Serbia.
# See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

# ===== COMPONENT SETUP =====

find_package(SFML CONFIG REQUIRED)

set(COMPONENT_NAME "Hobgoblin.UWGA")

# ===== GATHER SOURCES ======

set(COMPONENT_SOURCES
    "Source/System.cpp"

    # SFML
    "Source/SFML/Image_impl.cpp"
    "Source/SFML/Image_impl.hpp"
    "Source/SFML/Render_window_impl.cpp"
    "Source/SFML/Render_window_impl.hpp"
    "Source/SFML/System_impl.hpp"
    "Source/SFML/Transform_impl.cpp"
    "Source/SFML/Vertex_array_impl.cpp"
    "Source/SFML/Vertex_array_impl.hpp"
    "Source/SFML/View_impl.cpp"
    "Source/SFML/View_impl.hpp"
)

# ===== TARGET SETUP =====

add_library(${COMPONENT_NAME} STATIC ${COMPONENT_SOURCES})
target_include_directories(${COMPONENT_NAME} PUBLIC "Include/")

target_link_libraries(${COMPONENT_NAME}
PUBLIC
    # Foundation
    "Hobgoblin_L00_S00_PDef"
    "Hobgoblin_L00_S01_Common"
    "Hobgoblin_L00_S02_HGExcept"
    "Hobgoblin_L00_S03_Logging"
    "Hobgoblin_L00_S04_Unicode"

    # Utilities
    "Hobgoblin_L01_S00_Math"
    "Hobgoblin_L01_S02_Utility"

    # Principals
    "Hobgoblin_L02_S00_Window"

    # External
    "sfml::sfml"
)

add_library("Hobgoblin_L02_S01_UWGA" ALIAS ${COMPONENT_NAME})

add_subdirectory("Test")
