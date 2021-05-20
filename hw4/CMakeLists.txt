cmake_minimum_required(VERSION 3.19)
project(hw4)
#add_compile_options(-Wall -Wextra -pedantic -Werror  )
add_compile_options(-DEBUG )
set(CMAKE_EXE_LINKER_FLAGS "-lrt -lpthread")
set(CMAKE_C_STANDARD 11)

add_executable(hw4
        main.c queue.c)
