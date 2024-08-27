cmake_minimum_required(VERSION 3.12)

include(pico_sdk_import.cmake)

pico_sdk_init()

project(tetris C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

set(PICOSYSTEM_DIR ${CMAKE_CURRENT_LIST_DIR}/picosystem)
find_package(PICOSYSTEM REQUIRED)

picosystem_executable(
  tetris
  tetris.cpp
)

target_link_libraries(
  tetris
  pico_stdlib
  pico_rand
)

disable_startup_logo(tetris)
no_spritesheet(tetris)
