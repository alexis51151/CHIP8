cmake_minimum_required(VERSION 3.25)

project(CHIP8)

set(CMAKE_CXX_STANDARD 20)

target_sources(
  PRIVATE
    chip.cpp
)

add_executable(CHIP8)
