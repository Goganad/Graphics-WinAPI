cmake_minimum_required(VERSION 3.17)
project(Sprite_WINAPI)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -municode")
set(CMAKE_CXX_STANDARD_LIBRARIES "${CMAKE_CXX_STANDARD_LIBRARIES} -lgdiplus") #target_link_libraries(Sprite_WINAPI -lgdiplus)

add_executable(Sprite_WINAPI main.cpp FloatingImage.h)