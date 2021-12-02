# ******************************************************************************
#
#   Tonewheel Audio Engine
#
#   Copyright (C) 2021 Arthur Benilov <arthur.benilov@gmail.com>
#
# ******************************************************************************

set(src "${CMAKE_CURRENT_SOURCE_DIR}/googletest/src/gtest-all.cc")

add_library(gtest STATIC ${src})
target_include_directories(gtest PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/googletest/include")
target_include_directories(gtest PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/googletest")

set_target_properties(gtest PROPERTIES main_cc "${CMAKE_CURRENT_SOURCE_DIR}/googletest/src/gtest_main.cc")
