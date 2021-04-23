cmake_minimum_required(VERSION 3.10)

project(main)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic -Werror")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -fsanitize=leak -fsanitize=undefined")

set(SRC_LIST main.cpp)

add_executable(main ${SRC_LIST})

target_link_libraries(main ${CMAKE_SOURCE_DIR}/../lib/liblz4.a)
target_link_libraries(main ${CMAKE_SOURCE_DIR}/../lib/libzstd.a)
