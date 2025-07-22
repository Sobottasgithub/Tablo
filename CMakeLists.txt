cmake_minimum_required(VERSION 3.10)
project(tablo)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Tablo shared libraries

# libtabcrypt
add_subdirectory(lib/libtabcrypt)



# TabloClient, TabloMaster and TabloNode executables (provided by own CMakeLists.txt files)

add_subdirectory(TabloClient)
add_subdirectory(TabloMaster)
add_subdirectory(TabloNode)
