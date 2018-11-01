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
file(GLOB opt_SRCS ${opt_DIR}/*.c)
file(GLOB cli_SRCS ${cli_DIR}/*.c)
file(GLOB module_SRCS ${module_DIR}/*.c)
file(GLOB vm_SRCS ${vm_DIR}/*.c)
file(GLOB API_test_SRCS ${API_test_DIR}/*.c)
file(GLOB unit_test_SRCS ${unit_test_DIR}/*.c)

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
set(LibUV_BIN_DIR ${CMAKE_CURRENT_BINARY_DIR}/LibUV-prefix/src/LibUV/out/Release)

add_library(libuv_static STATIC IMPORTED)
set_target_properties(libuv_static PROPERTIES
    IMPORTED_LOCATION ${LibUV_BIN_DIR}/libuv.a
    INTERFACE_INCLUDE_DIRECTORIES ${LibUV_SRC_DIR}/include
    INTERFACE_COMPILE_OPTIONS -pthread
    INTERFACE_LINK_LIBRARIES -pthread
)

add_dependencies(libuv_static LibUV)
add_library(wren_vm OBJECT ${vm_SRCS})
target_include_directories(wren_vm 
    PUBLIC src/include
    PRIVATE src/optional src/vm
)

add_library(wren_opt OBJECT ${opt_SRCS})
target_include_directories(wren_opt
    PUBLIC src/include
    PRIVATE src/optional src/vm
)

add_library(wren STATIC $<TARGET_OBJECTS:wren_vm> $<TARGET_OBJECTS:wren_opt>)
target_include_directories(wren INTERFACE src/include)

add_executable(wren_cli ${cli_SRCS} ${module_SRCS})
target_include_directories(wren_cli 
    PRIVATE src/cli src/module
)
target_link_libraries(wren_cli PRIVATE wren m libuv_static)