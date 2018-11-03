# Need transitive dependencies introduced in 2.8.12
cmake_minimum_required(VERSION 2.8.12)
option(ENABLE_TESTING "Build Tests" ON)
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

set(wren_SRCS ${vm_SRCS} ${opt_SRCS})
set(wren_CLI_SRCS ${cli_SRCS} ${module_SRCS})
set(wren_API_test_SRCS 
    ${API_test_SRCS}
    ${module_SRCS}
    ${cli_DIR}/modules.c
    ${cli_DIR}/vm.c
    ${cli_DIR}/path.c
)
set(wren_unit_test_SRCS
    ${unit_test_SRCS}
    ${module_SRCS}
    ${cli_DIR}/modules.c
    ${cli_DIR}/vm.c
    ${cli_DIR}/path.c
)

# Linux Only for now
include(ExternalProject)
add_subdirectory(deps/libuv)
function(copy_to_dir target dest)
    set(test_bin_dir ${CMAKE_CURRENT_SOURCE_DIR}/bin)
    file(MAKE_DIRECTORY ${dest})
    add_custom_command(TARGET ${target} POST_BUILD
        COMMAND cp $<TARGET_FILE:${target}> ${dest}/
    )
endfunction()


add_library(wren STATIC ${wren_SRCS})
target_include_directories(wren 
    PUBLIC src/include
    PRIVATE src/vm src/optional)
target_link_libraries(wren
    INTERFACE m
)
target_compile_options(wren 
    PRIVATE "-Wall" "-Wextra" "-Werror" "-Wno-unused-parameter"
)
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_options(wren 
        PUBLIC -DDEBUG
    )
endif()

add_executable(wren_cli ${wren_CLI_SRCS})
copy_to_dir(wren_cli ${CMAKE_CURRENT_SOURCE_DIR}/bin)
target_include_directories(wren_cli 
    PRIVATE src/cli src/module
)
target_link_libraries(wren_cli PRIVATE wren uv_a)
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    set_target_properties(wren_cli PROPERTIES
        OUTPUT_NAME wrend
    )
else()
    set_target_properties(wren_cli PROPERTIES
        OUTPUT_NAME wren
    )
endif()

if (ENABLE_TESTING)
    enable_testing()
    add_executable(wren_API_test ${wren_API_test_SRCS})
    set_target_properties(wren_API_test PROPERTIES
        OUTPUT_NAME "api_wrend"
    )
    copy_to_dir(wren_API_test ${CMAKE_CURRENT_BINARY_DIR}/debug/test)
    target_include_directories(wren_API_test
        PRIVATE src/cli src/module
    )
    target_link_libraries(wren_API_test
        PRIVATE wren uv_a)

    add_executable(wren_unit_test ${wren_unit_test_SRCS})
    target_include_directories(wren_unit_test
        PRIVATE src/cli src/module
    )
    target_link_libraries(wren_unit_test
        PRIVATE wren uv_a
    )

    add_test(NAME UnitTest 
        COMMAND wren_unit_test
    )
endif()