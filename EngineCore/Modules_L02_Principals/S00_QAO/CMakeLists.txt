# Copyright 2024 Jovan Batnozic. Released under MS-PL licence in Serbia.
# See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

# ===== COMPONENT SETUP =====

set(COMPONENT_NAME "Hobgoblin.QAO")

# ===== GATHER SOURCES ======

set(COMPONENT_SOURCES
  "Source/Base.cpp"
  "Source/Functions_destroy.cpp"
  "Source/Handle.cpp"
  "Source/Id.cpp"
  "Source/Orderer.cpp"
  "Source/Priority_resolver.cpp"
  "Source/Priority_resolver2.cpp"
  "Source/Registry.cpp"
  "Source/Runtime.cpp"
)

# ===== TARGET SETUP =====

add_library(${COMPONENT_NAME} STATIC ${COMPONENT_SOURCES})
target_include_directories(${COMPONENT_NAME} PUBLIC "Include/")

target_link_libraries(${COMPONENT_NAME}
PUBLIC
    # Foundation
    "Hobgoblin_L00_S00_PDef"
    "Hobgoblin_L00_S02_HGExcept"

    # Utilities
    "Hobgoblin_L00_S01_Common"
    "Hobgoblin_L01_S02_Utility"
)

add_library("Hobgoblin_L02_S00_QAO" ALIAS ${COMPONENT_NAME})

add_subdirectory("Test")
