
option(WAUXLIB_LOADER "Build Wauxlib loader" ON)
option(WAUXLIB_BINDER "Build Wauxlib binder" ON)


set(wauxlib_SRCS)
set(wauxlib_LIBS wren)

if (WAUXLIB_LOADER)
    list(APPEND wauxlib_SRCS "src/wauxlib_loader.c")
endif()

if (WAUXLIB_BINDER)
    add_subdirectory(deps/klib)
    list(APPEND wauxlib_LIBS klib)
    list(APPEND wauxlib_SRCS "src/wauxlib_binder.c")
endif()

add_library(wauxlib STATIC ${wauxlib_SRCS})
target_include_directories(wauxlib PUBLIC "include")
target_compile_options(wauxlib PRIVATE "-Wall" "-Wextra" "-Werror")
target_link_libraries(wauxlib PUBLIC ${wauxlib_LIBS})