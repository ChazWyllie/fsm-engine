cmake_minimum_required(VERSION 3.20)
project(fsm_engine CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Strict warnings — mirrors defense coding standards.
add_compile_options(
    -Wall
    -Wextra
    -Wpedantic
    -Wshadow
    -Wno-unused-parameter
)

# FSM library (no dynamic allocation after init — REQ-007).
add_library(fsm STATIC src/FSM.cpp)
target_include_directories(fsm PUBLIC include)

# Example demo binary.
add_executable(effector_demo examples/effector_demo.cpp)
target_link_libraries(effector_demo PRIVATE fsm)

# Tests.
enable_testing()
find_package(GTest REQUIRED)

add_executable(test_fsm tests/test_fsm.cpp)
target_link_libraries(test_fsm PRIVATE fsm GTest::gtest_main)

add_test(NAME FSMTests COMMAND test_fsm)
