cmake_minimum_required(VERSION 3.13)
project(hw2 C)

set(CMAKE_C_STANDARD 99)
add_executable(hw2 main.c signal_handler.c lagrange.c file_locking.c)