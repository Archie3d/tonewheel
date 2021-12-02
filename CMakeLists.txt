# ******************************************************************************
#
#   Tonewheel Audio Engine
#
#   Copyright (C) 2021 Arthur Benilov <arthur.benilov@gmail.com>
#
# ******************************************************************************

cmake_minimum_required(VERSION 3.19)

project(tonewheel VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 20)

enable_testing()

add_subdirectory(source)
add_subdirectory(tests)
