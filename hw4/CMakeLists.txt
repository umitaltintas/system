cmake_minimum_required(VERSION 3.19)
project(hw4)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(PROJECT_DIR ${CMAKE_SOURCE_DIR})

set(PROJECT_INCLUDE_DIR ${PROJECT_DIR}/lib)


set(PROJECT_SOURCE_DIR ${PROJECT_DIR}/src)


set( PROJECT_SOURCES
        src/main.c
        src/data.c
        src/queue.c
        src/sort.c)

set(PROJECT_HEADERS
        $(PROJECT_INCLUDE_DIR)/main.h
        $(PROJECT_INCLUDE_DIR)/sort.h
        $(PROJECT_INCLUDE_DIR)/queue.h
        $(PROJECT_INCLUDE_DIR)/data.h
        )

include_directories(  ${PROJECT_INCLUDE_DIR} )
add_compile_options(-Wall -Wextra -pedantic -Werror  )

set(CMAKE_EXE_LINKER_FLAGS "-lrt -lpthread")
set(CMAKE_C_STANDARD 11)

add_executable(app ${PROJECT_SOURCES})
