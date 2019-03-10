
option(WAUXLIB_LOADER "Build Wauxlib loader" ON)
option(WAUXLIB_BINDER "Build Wauxlib binder" ON)

set(wauxlib_SRCS)

if (WAUXLIB_LOADER)
    list(APPEND wauxlib_SRCS "src/wauxlib_loader.c")
endif()

if (WAUXLIB_BINDER)
    list(APPEND wauxlib_SRCS "src/wauxlib_binder.c")
endif()

add_library(wauxlib STATIC ${wauxlib_SRCS})
target_include_directories(wauxlib PUBLIC "include")
target_compile_options(wauxlib PRIVATE "-Wall" "-Wextra" "-Werror")
target_link_libraries(wauxlib PUBLIC wren)