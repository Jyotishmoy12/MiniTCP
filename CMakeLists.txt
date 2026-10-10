cmake_minimum_required(VERSION 3.20)  # Set the minimum required version of CMake to 3.20

project(MiniTCP
    VERSION 0.1.0  # Set the project name to "MiniTCP" and the version to 0.1.0
    LANGUAGES CXX # Specify that the project uses the C++ programming language
)   

set(CMAKE_CXX_STANDARD 20)  # Set the C++ standard to C++20
set(CMAKE_CXX_STANDARD_REQUIRED ON)  # Ensure that the specified C++ standard is required
set(CMAKE_CXX_EXTENSIONS OFF)  # Disable compiler-specific extensions to ensure portability

enable_testing() # Enable testing for the project

add_executable(minitcp  # Create an executable target named "minitcp"
     src/main.cpp
)

target_compile_features(minitcp PRIVATE cxx_std_20)  # Specify that the "minitcp" target requires C++20 features

target_include_directories(minitcp PRIVATE 
          include
)    

add_executable(minitcp_tests
     tests/test_main.cpp
)


target_compile_features(minitcp_tests PRIVATE cxx_std_20)

target_include_directories(minitcp_tests PRIVATE
    include
)

add_test(NAME minitcp_tests COMMAND minitcp_tests)  # Add a test named "minitcp_tests" that runs the "minitcp_tests" executable