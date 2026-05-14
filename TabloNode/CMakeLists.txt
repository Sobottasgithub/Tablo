cmake_minimum_required(VERSION 3.10)
project(tablo)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB_RECURSE SOURCES "src/*.cpp" "src/*.h")

add_executable(tablo-node ${SOURCES})

# Link against libs
target_link_libraries(tablo-node PRIVATE tabcrypt)
target_link_libraries(tablo-node PRIVATE tabnet)
target_link_libraries(tablo-node PRIVATE ttp2)

install(TARGETS tablo-node DESTINATION bin)
