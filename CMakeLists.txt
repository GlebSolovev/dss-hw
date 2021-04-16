cmake_minimum_required(VERSION 3.10)

project(main)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")

set(SMHASHER_SRC ../smhasher/src/SpeedTest.cpp ../smhasher/src/MurmurHash3.cpp)
set(SPOOKY_SRC ../SpookyHash/SpookyV2.cpp)
set(SRC_LIST main.cpp ${SMHASHER_SRC} ${SPOOKY_SRC})

include_directories(../smhasher/src/)
include_directories(../SpookyHash/)

add_executable(main ${SRC_LIST})

add_subdirectory(../xxHash/cmake_unofficial/ ../../xxHash/build/ EXCLUDE_FROM_ALL)

target_link_libraries(main PRIVATE xxHash::xxhash)
