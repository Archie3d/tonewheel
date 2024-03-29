# ******************************************************************************
#
#   Tonewheel Audio Engine
#
#   Copyright (C) 2021 Arthur Benilov <arthur.benilov@gmail.com>
#
# ******************************************************************************

set(target tonewheel)

file(GLOB_RECURSE src
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
)

add_library(${target} STATIC ${src})

set_property(TARGET ${target} PROPERTY CXX_STANDARD 20)

if(APPLE)
    if (${CMAKE_HOST_SYSTEM_PROCESSOR} MATCHES "x86_64")
        target_compile_definitions(${target}
            PUBLIC
                -mfma -msse -msse3 -msse4.1 -msse4.2 -mssse3 -mavx -mavx2
        )
    endif()
elseif(MSVC)
    target_compile_options(${target}
        PUBLIC "/bigobj"
    )
endif()

if(CMAKE_BUILD_TYPE MATCHES "Debug")
    target_compile_definitions(${target} PUBLIC -DDEBUG)
endif()

target_include_directories(${target} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/../")
target_include_directories(${target} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")

target_link_libraries(${target}
    PUBLIC
        exprtk
        libogg
        libvorbis
)

if (TONEWHEEL_WITH_OPUS)
    target_link_libraries(${target} PUBLIC libopus opusfile)
endif()
