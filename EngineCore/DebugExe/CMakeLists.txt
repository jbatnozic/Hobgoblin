# Copyright 2024 Jovan Batnozic. Released under MS-PL licence in Serbia.
# See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence


find_package(SFML CONFIG REQUIRED)

add_executable("Hobgoblin.DebugExe"
  "Source/RcssPreprocessorMain.cpp"
)

target_link_libraries("Hobgoblin.DebugExe"
PUBLIC
  "Hobgoblin.Common"
  "Hobgoblin.Graphics"
  "Hobgoblin.Logging"
  # "Hobgoblin.QAO"
  # "Hobgoblin.RigelNet"
  "Hobgoblin.Math"
  "Hobgoblin.Utility"
  "Hobgoblin.RmlUi"
  "sfml::sfml"
  # "ztcpp::ztcpp"
)
