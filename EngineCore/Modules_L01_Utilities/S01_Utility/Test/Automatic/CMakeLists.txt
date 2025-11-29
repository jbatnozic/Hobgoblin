# Copyright 2024 Jovan Batnozic. Released under MS-PL licence in Serbia.
# See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

project("Hobgoblin.Utility.AutomaticTest")

find_package(GTest CONFIG REQUIRED)

add_executable(${PROJECT_NAME}
    "Base64_test.cpp"
    "Compact_int_test.cpp"
    "Compressed_small_vector_test.cpp"
    "Packet_test.cpp"
    "Stream_out_adapter_test.cpp"
    "Value_sorted_map_test.cpp"
)

target_link_libraries(${PROJECT_NAME}
PUBLIC
    # Utilities
    "Hobgoblin.Utility"

    # External
    "gtest::gtest"
)

include(GoogleTest)
gtest_discover_tests("Hobgoblin.Utility.AutomaticTest")
