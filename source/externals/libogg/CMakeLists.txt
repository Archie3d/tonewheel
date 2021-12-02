# ******************************************************************************
#
#   Tonewheel Audio Engine
#
#   Copyright (C) 2021 Arthur Benilov <arthur.benilov@gmail.com>
#
# ******************************************************************************

file(GLOB src "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c")
add_library(libogg STATIC ${src})
target_include_directories(libogg PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
