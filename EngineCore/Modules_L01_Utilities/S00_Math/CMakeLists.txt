# Copyright 2024 Jovan Batnozic. Released under MS-PL licence in Serbia.
# See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

# ===== COMPONENT SETUP =====

find_package(SFML CONFIG REQUIRED)

set(COMPONENT_NAME "Hobgoblin.Math")

# ===== GATHER SOURCES ======

set(COMPONENT_SOURCES
    "Source/Hash.cpp"
)

# ===== TARGET SETUP =====

add_library(${COMPONENT_NAME} STATIC ${COMPONENT_SOURCES})
target_include_directories(${COMPONENT_NAME} PUBLIC "Include/")

target_link_libraries(${COMPONENT_NAME}
PUBLIC
    # Foudnation
    "Hobgoblin_L00_S01_Common"

    # Enternal
    "sfml::sfml"
)

add_library("Hobgoblin_L01_S00_Math" ALIAS ${COMPONENT_NAME})

add_subdirectory("Test")
