cmake_minimum_required(VERSION 3.10)
project(tablo)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB_RECURSE SOURCES "src/*.cpp" "src/*.h")

add_executable(tablo-node ${SOURCES})

# Link against libs
target_link_libraries(tablo-node PRIVATE tabcrypt)
target_link_libraries(tablo-node PRIVATE ttp2)
target_link_libraries(tablo-node PRIVATE tud)
find_package(Arrow REQUIRED)
target_link_libraries(tablo-node PRIVATE Arrow::arrow_shared)

install(TARGETS tablo-node DESTINATION bin)
