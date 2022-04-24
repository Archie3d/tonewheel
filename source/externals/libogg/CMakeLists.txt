# ******************************************************************************
#
#   Tonewheel Audio Engine
#
#   Copyright (C) 2021 Arthur Benilov <arthur.benilov@gmail.com>
#
# ******************************************************************************

set(TARGET libogg)

file(GLOB src "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c")
add_library(${TARGET} STATIC ${src})
target_include_directories(${TARGET} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
