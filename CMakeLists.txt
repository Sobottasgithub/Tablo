cmake_minimum_required(VERSION 3.10)
project(tablo)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


# TabloClient, TabloMaster and TabloNode executables

# TabloClient
file(GLOB_RECURSE TABLO_CLIENT_SOURCES "src/TabloClient/*.cpp")
file(GLOB_RECURSE TABLO_CLIENT_HEADERS "include/TabloClient/*.h")