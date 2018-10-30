# Need transitive dependencies introduced in 2.8.12
cmake_minimum_required(VERSION 2.8.12)
project(Wren LANGUAGES "C")

set(wren_warning_flags "-Wall"
                       "-Wextra"
                       "-Werror"
                       "-Wno-unused-parameter"
)           
set(opt_DIR "src/optional")
set(cli_DIR "src/cli")
set(module_DIR "src/module")
set(vm_DIR "src/vm")
set(API_test_DIR "test/api")
set(unit_test_DIR "test/unit")
# TODO: Don't glob
file(GLOB opt_SRCS RELATIVE ${opt_DIR} *.c)
file(GLOB cli_SRCS RELATIVE ${cli_DIR} *.c)
file(GLOB module_SRCS RELATIVE ${module_DIR} *.c)
file(GLOB vm_SRCS RELATIVE ${vm_DIR} *.c)
file(GLOB API_test_SRCS RELATIVE ${API_test_DIR} *.c)
file(GLOB unit_test_SRCS RELATIVE ${unit_test_DIR} *.c)

# Linux Only for now
include(ExternalProject)
set(LibUV_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/deps/libuv")
ExternalProject_Add(
    "LibUV"
    URL "${LibUV_SRC_DIR}"
    CONFIGURE_COMMAND python2 gyp_uv.py -f make
    BUILD_IN_SOURCE True
    BUILD_COMMAND BUILDTYPE=Release make -C out libuv.a
    INSTALL_COMMAND ""
)