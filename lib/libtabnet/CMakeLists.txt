cmake_minimum_required(VERSION 3.10)
project(tabnet)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


# find_library(protobuf protobuf)

find_package(protobuf REQUIRED)

include_directories(
  include
  ${Protobuf_INCLUDE_DIRS}
)

file(GLOB_RECURSE SOURCES "src/*.cpp")
# file(GLOB_RECURSE HEADERS "include/*.h")

set(PROTO_FILES
  src/protobuf/transfere_protocol.proto
)

protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS ${PROTO_FILES})

add_library(tabnet SHARED ${SOURCES} ${PROTO_SRCS} ${PROTO_HDRS})

target_include_directories(tabnet PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_BINARY_DIR} # for protobuf
)

# Link against the tabcrypt and protobuf library
target_link_libraries(tabnet PRIVATE tabcrypt ${protobuf})

install(TARGETS tabnet DESTINATION lib)
install(DIRECTORY include/ DESTINATION include)
