# Copyright 2024 Jovan Batnozic. Released under MS-PL licence in Serbia.
# See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

# ===== COMPONENT SETUP =====

# The 'glu' Conan package resolves to a system package on some platforms (e.g. macOS)
# and does not declare a 'glu::glu' target, yet GLEW's link interface references it.
# GLU is provided via the OpenGL framework (opengl::opengl), so a stub target is enough.
if(NOT TARGET glu::glu)
    add_library(glu::glu INTERFACE IMPORTED)
endif()

find_package(glew CONFIG REQUIRED)
find_package(RmlUi CONFIG REQUIRED)
find_package(SFML CONFIG REQUIRED)

set(COMPONENT_NAME "Hobgoblin.RmlUi")

# ===== GATHER SOURCES ======

set(COMPONENT_SOURCES
    "Source/Context_driver.cpp"
    "Source/Hobgoblin_backend.cpp"
    "Source/RmlUi_Hobgoblin_renderer.cpp"
    "Source/RmlUi_Hobgoblin_system.cpp"
    "Source/RmlUi_preprocessing.cpp"
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

    # Utilities
    "Hobgoblin_L01_S01_Utility"

    # Principals
    "Hobgoblin_L02_S00_Input"
    "Hobgoblin_L02_S01_UWGA"

    # External
    "GLEW::GLEW"
    "RmlUi::RmlUi"
    "sfml::sfml"
)

add_library("Hobgoblin_L02_S02_RmlUi" ALIAS ${COMPONENT_NAME})
