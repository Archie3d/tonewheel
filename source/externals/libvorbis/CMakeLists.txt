# ******************************************************************************
#
#   Tonewheel Audio Engine
#
#   Copyright (C) 2021 Arthur Benilov <arthur.benilov@gmail.com>
#
# ******************************************************************************

file(GLOB src
    "${CMAKE_CURRENT_SOURCE_DIR}/vorbis/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c"
)

add_library(libvorbis STATIC ${src})
target_include_directories(libvorbis PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
target_include_directories(libvorbis PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src")

target_link_libraries(libvorbis PRIVATE libogg)

if(WIN32)
    target_compile_options(libvorbis PRIVATE "/wd4244")
    target_compile_options(libvorbis PRIVATE "/wd4267")
    target_compile_options(libvorbis PRIVATE "/wd4996")
    target_compile_options(libvorbis PRIVATE "/wd4305")
endif()
