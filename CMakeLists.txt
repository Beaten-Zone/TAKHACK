cmake_minimum_required(VERSION 3.16)
project(takhack CXX)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_subdirectory(examples/cot-udp/cpp)
