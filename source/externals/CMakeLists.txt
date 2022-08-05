# ******************************************************************************
#
#   Tonewheel Audio Engine
#
#   Copyright (C) 2021 Arthur Benilov <arthur.benilov@gmail.com>
#
# ******************************************************************************

add_subdirectory(exprtk)
add_subdirectory(libogg)
add_subdirectory(libvorbis)

if (TONEWHEEL_WITH_OPUS)
    add_subdirectory(libopus)
    add_subdirectory(opusfile)
endif()
