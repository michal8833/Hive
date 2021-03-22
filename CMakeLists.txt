cmake_minimum_required(VERSION 3.17)
project(hive)
set(CMAKE_CXX_STANDARD 20)

add_library(utils
        utils/include/defs.h
        utils/include/Flower.h utils/include/Flowerbed.h
        utils/src/Flower.cpp utils/src/Flowerbed.cpp)

target_include_directories(utils PUBLIC utils/include)

add_executable(hive src/hive.cpp)
add_executable(queen src/queen.cpp)
add_executable(bee src/bee.cpp)

target_link_libraries(hive LINK_PRIVATE utils)
target_link_libraries(queen LINK_PRIVATE utils)
target_link_libraries(bee LINK_PRIVATE utils)
