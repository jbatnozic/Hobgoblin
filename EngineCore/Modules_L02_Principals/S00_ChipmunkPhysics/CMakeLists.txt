# Copyright 2024 Jovan Batnozic. Released under MS-PL licence in Serbia.
# See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

# ===== COMPONENT SETUP =====

find_package(SFML CONFIG REQUIRED)

set(COMPONENT_NAME "Hobgoblin.ChipmunkPhysics")

# ===== GATHER SOURCES ======

set(COMPONENT_SOURCES
    "Source/cpSpace.c"
    "Source/cpSpaceStep.c"
    "Source/cpSpaceDebug.c"
    "Source/cpGearJoint.c"
    "Source/cpGrooveJoint.c"
    "Source/cpSpaceQuery.c"
    "Source/cpHashSet.c"
    "Source/cpDampedRotarySpring.c"
    "Source/cpBody.c"
    "Source/cpSweep1D.c"
    "Source/cpShape.c"
    "Source/cpRobust.c"
    "Source/cpPinJoint.c"
    "Source/cpPivotJoint.c"
    "Source/cpCollision.c"
    "Source/cpBBTree.c"
    "Source/cpArray.c"
    "Source/cpMarch.c"
    "Source/cpPolyline.c"
    "Source/chipmunk.c"
    "Source/cpDampedSpring.c"
    "Source/cpSpaceComponent.c"
    "Source/cpConstraint.c"
    "Source/cpPolyShape.c"
    "Source/cpSpaceHash.c"
    "Source/cpArbiter.c"
    "Source/cpSimpleMotor.c"
    "Source/cpRotaryLimitJoint.c"
    "Source/cpSpatialIndex.c"
    "Source/cpRatchetJoint.c"
    "Source/cpSlideJoint.c"
    "Source/cpHastySpace.c"
)

# Force Chipmunk to be compiled as C++
set_source_files_properties(${COMPONENT_SOURCES} PROPERTIES LANGUAGE CXX)

# ===== TARGET SETUP =====

add_library(${COMPONENT_NAME} STATIC ${COMPONENT_SOURCES})
target_include_directories(${COMPONENT_NAME} PUBLIC "Include/")
target_include_directories(${COMPONENT_NAME} PUBLIC "Include/Hobgoblin/Private")

target_link_libraries(${COMPONENT_NAME}
PUBLIC
    # Foundation
    "Hobgoblin_L00_S00_PDef"

    # Utilities
    "Hobgoblin_L01_S02_Utility"
)

add_library("Hobgoblin_L02_S00_ChipmunkPhysics" ALIAS ${COMPONENT_NAME})
