# ******************************************************************************
#
#   Tonewheel Audio Engine
#
#   Copyright (C) 2022 Arthur Benilov <arthur.benilov@gmail.com>
#
# ******************************************************************************

set(TARGET opusfile)

file(GLOB src
    "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c"
)

add_library(${TARGET} STATIC ${src})
target_include_directories(${TARGET} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")

target_link_libraries(${TARGET}
    PRIVATE
        libogg
        libopus
)
