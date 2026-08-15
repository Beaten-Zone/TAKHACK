cmake_minimum_required(VERSION 3.16)
project(cot_udp_examples CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(send_cot send_cot.cpp)

find_package(Threads REQUIRED)
target_link_libraries(send_cot PRIVATE Threads::Threads)

target_compile_options(send_cot PRIVATE -Wall -Wextra)
