cmake_minimum_required(VERSION 2.8.12)

# TODO: Add non-header only part of klib
add_library(klib INTERFACE)
target_include_directories(klib INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})