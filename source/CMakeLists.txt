# ******************************************************************************
#
#   Tonewheel Audio Engine
#
#   Copyright (C) 2021 Arthur Benilov <arthur.benilov@gmail.com>
#
# ******************************************************************************

option(TONEWHEEL_WITH_OPUS "Enable opus audio codec support" OFF)

if (TONEWHEEL_WITH_OPUS)
    add_definitions(TONEWHEEL_WITH_OPUS=1)
endif()

add_subdirectory(externals)
add_subdirectory(engine)
