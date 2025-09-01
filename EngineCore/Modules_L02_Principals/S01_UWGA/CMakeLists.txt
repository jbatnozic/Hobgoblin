# Copyright 2025 Jovan Batnozic. Released under MS-PL licence in Serbia.
# See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

# ===== COMPONENT SETUP =====

find_package(SFML CONFIG REQUIRED)

set(COMPONENT_NAME "Hobgoblin.UWGA")

# ===== GATHER SOURCES ======

set(COMPONENT_SOURCES
    "Source/Circle_shape.cpp"
    "Source/Opengl_utils.cpp"
    "Source/Rectangle_shape.cpp"
    "Source/Shape.cpp"
    "Source/Sprite_attributes.cpp"
    "Source/Sprite_blueprint.cpp"
    "Source/Sprite_loader.cpp"
    "Source/Sprite_manifest_parser.cpp"
    "Source/Sprite.cpp"
    "Source/System.cpp"
    "Source/Text.cpp"
    "Source/Texture_packing.cpp"
    "Source/Transformable.cpp"
    "Source/Vertex_array.cpp"

    # BuiltinFonts
    "Source/BuiltinFonts/Font_cmu_serif_roman.cpp"
    "Source/BuiltinFonts/Font_cmu_typewriter_text.cpp"
    "Source/BuiltinFonts/Font_ebgaramond12_regular.cpp"
    "Source/BuiltinFonts/Font_inconsolata_regular.cpp"
    "Source/BuiltinFonts/Font_titillium_webregular.cpp"

    # SFML
    "Source/SFML/Font_impl.cpp"
    "Source/SFML/Font_impl.hpp"
    "Source/SFML/Glsl_shader_impl.cpp"
    "Source/SFML/Glsl_shader_impl.hpp"
    "Source/SFML/Image_impl.cpp"
    "Source/SFML/Image_impl.hpp"
    "Source/SFML/Render_window_impl.cpp"
    "Source/SFML/Render_window_impl.hpp"
    "Source/SFML/System_impl.hpp"
    "Source/SFML/Text_impl.cpp"
    "Source/SFML/Text_impl.hpp"
    "Source/SFML/Transform_impl.cpp"
    "Source/SFML/View_impl.cpp"
    "Source/SFML/View_impl.hpp"

    # MaxRectsBinPack
    "Source/MaxRectsBinPack/Max_rects_bin_pack.cpp"
    "Source/MaxRectsBinPack/Rbp_rect.cpp"
)

# ===== TARGET SETUP =====

add_library(${COMPONENT_NAME} STATIC ${COMPONENT_SOURCES})

target_include_directories(${COMPONENT_NAME}
PUBLIC
    "Include/"

PRIVATE
    "Source/"
)

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
