# Need transitive dependencies introduced in 2.8.12
cmake_minimum_required(VERSION 2.8.12)
project(Wren LANGUAGES "C")
if (${CMAKE_PROJECT_NAME} STREQUAL "Wren")
    option(WREN_BUILD_CLI "Build CLI" ON)
    option(ENABLE_TESTING "Build Tests" ON)
else()
    option(WREN_BUILD_CLI "Build CLI" OFF)
    option(ENABLE_TESTING "Build Tests" OFF)
endif()
option(WREN_BUILD_WAUXLIB "Build Wauxlib" ON)
option(WREN_SUPPORT_PLUGINS "Support Wren Plugins" ON)

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

if(WREN_SUPPORT_PLUGINS)
    add_library(wren_plugin_interface INTERFACE)
    target_include_directories(wren_plugin_interface INTERFACE src/include)
    #INTERFACE_LINK_OPTIONS doesn't exist until CMake 3.13 and don't want force dep on that new
    #So we use this hack
    get_target_property(wren_interface_libs wren INTERFACE_LINK_LIBRARIES)
    list(APPEND wren_interface_libs "-Wl,--dynamic-list=\"${CMAKE_CURRENT_SOURCE_DIR}/wren_exported_symbols.txt\"")
    set_target_properties(wren PROPERTIES
        INTERFACE_LINK_LIBRARIES "${wren_interface_libs}"
    )
endif()

if(WREN_BUILD_WAUXLIB)
    add_subdirectory(src/wauxlib)
endif()

if(WREN_BUILD_CLI)
    include(ExternalProject)
    add_subdirectory(deps/libuv)
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

        add_test(NAME Unit_Test 
            COMMAND wren_unit_test
        )

        add_test(NAME API_Test
            COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/util/test.py
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        )
    endif()
endif()