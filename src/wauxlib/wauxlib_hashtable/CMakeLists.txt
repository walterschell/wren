set(whashtable_SRCS src/whashtable.c)

add_library(whashtable STATIC ${whashtable_SRCS})
target_compile_options(whashtable PRIVATE "-Wall" "-Wextra" "-Werror")
target_include_directories(whashtable PUBLIC include)