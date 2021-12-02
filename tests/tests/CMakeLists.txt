# ******************************************************************************
#
#   Tonewheel Audio Engine
#
#   Copyright (C) 2021 Arthur Benilov <arthur.benilov@gmail.com>
#
# ******************************************************************************

file(GLOB_RECURSE src "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

get_target_property(gtest_main_cc gtest main_cc)
list(APPEND src ${gtest_main_cc})

add_executable(unit_tests ${src})

target_link_libraries(unit_tests PRIVATE
    gtest
    tonewheel
)

add_test(NAME unit_tests COMMAND unit_tests)
