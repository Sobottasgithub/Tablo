cmake_minimum_required(VERSION 3.10)
project(tablo)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB_RECURSE SOURCES "src/*.cpp" "src/*.h")

add_executable(tabcrypt-test ${SOURCES})

# Link against the tabcrypt library
target_link_libraries(tabcrypt-test PRIVATE tabcrypt)

install(TARGETS tabcrypt-test DESTINATION bin)
