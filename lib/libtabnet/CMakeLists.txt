cmake_minimum_required(VERSION 3.10)
project(tabnet)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB_RECURSE SOURCES "src/*.cpp")
# file(GLOB_RECURSE HEADERS "include/*.h")
# 
target_include_directories(tabnet PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

install(TARGETS tabnet DESTINATION lib)
install(DIRECTORY include/ DESTINATION include)
