cmake_minimum_required(VERSION 3.20)

project(MiniTCP
    VERSION 0.1.0
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

enable_testing()

add_library(minitcp_net
    src/net/checksum.cpp
)

target_compile_features(minitcp_net PUBLIC cxx_std_20)

target_include_directories(minitcp_net PUBLIC
    include
)

add_executable(minitcp
    src/main.cpp
)

target_compile_features(minitcp PRIVATE cxx_std_20)

target_link_libraries(minitcp PRIVATE
    minitcp_net
)

add_executable(minitcp_tests
    tests/test_main.cpp
)

target_compile_features(minitcp_tests PRIVATE cxx_std_20)

target_link_libraries(minitcp_tests PRIVATE
    minitcp_net
)

add_test(NAME minitcp_tests COMMAND minitcp_tests)