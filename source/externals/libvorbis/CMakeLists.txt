# ******************************************************************************
#
#   Tonewheel Audio Engine
#
#   Copyright (C) 2021 Arthur Benilov <arthur.benilov@gmail.com>
#
# ******************************************************************************

set(TARGET libvorbis)

file(GLOB src
    "${CMAKE_CURRENT_SOURCE_DIR}/vorbis/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c"
)

add_library(${TARGET} STATIC ${src})
target_include_directories(${TARGET} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
target_include_directories(${TARGET} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src")

target_link_libraries(${TARGET} PRIVATE libogg)

if(WIN32)
    target_compile_options(${TARGET} PRIVATE "/wd4244")
    target_compile_options(${TARGET} PRIVATE "/wd4267")
    target_compile_options(${TARGET} PRIVATE "/wd4996")
    target_compile_options(${TARGET} PRIVATE "/wd4305")
endif()
