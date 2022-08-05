# ******************************************************************************
#
#   Tonewheel Audio Engine
#
#   Copyright (C) 2022 Arthur Benilov <arthur.benilov@gmail.com>
#
# ******************************************************************************

set(TARGET libopus)

# Check CMAKE_OSX_ARCHITECTURES is "x86_64" or "arm64"

file(GLOB src_celt "${CMAKE_CURRENT_SOURCE_DIR}/celt/*.c")
file(GLOB src_celt_x86 "${CMAKE_CURRENT_SOURCE_DIR}/celt/x86/*.c")
file(GLOB src_silk "${CMAKE_CURRENT_SOURCE_DIR}/silk/*.c")
file(GLOB src_silk_x86 "${CMAKE_CURRENT_SOURCE_DIR}/silk/x86/*.c")
file(GLOB src "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c")

add_library(${TARGET} STATIC
    ${src}
    ${src_celt}
    ${src_silk}
)

target_include_directories(${TARGET}
    PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/include"
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/celt
        ${CMAKE_CURRENT_SOURCE_DIR}/silk
        ${CMAKE_CURRENT_SOURCE_DIR}/silk/float
)

target_compile_options(${TARGET} PRIVATE "-DHAVE_CONFIG_H")
