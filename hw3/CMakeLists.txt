cmake_minimum_required(VERSION 3.13)
project(hw3 C)
add_compile_options(-Wall -Wextra -pedantic -Werror  )
set(CMAKE_EXE_LINKER_FLAGS "-lrt -lpthread")
set(CMAKE_C_STANDARD 99)
add_executable(hw3 src/main.c src/usage.c)
