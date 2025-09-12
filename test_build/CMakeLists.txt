cmake_minimum_required(VERSION 3.16)
project(tulpar-physics-test VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable testing
enable_testing()

# Add test executable with new framework
add_executable(physics-tests
    ../tests/test_runner_simple.cpp
    ../tests/framework/TestManager.cpp
)

target_include_directories(physics-tests PRIVATE
    ..
)

# Register tests with CTest
add_test(NAME PhysicsEngineTests COMMAND physics-tests)