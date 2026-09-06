cmake_minimum_required(VERSION 3.20)
project(Live-Vision VERSION 0.1 LANGUAGES CXX)

include(FetchContent)

FetchContent_Declare(
googletest
GIT_REPOSITORY https://github.com/google/googletest.git
GIT_TAG  v1.18.0
)

FetchContent_MakeAvailable(googletest)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(LiveVision src/main.cpp)

add_executable(LiveVisionTests tests/test_main.cpp)

target_link_libraries(LiveVisionTests PRIVATE GTest::gtest_main)

enable_testing()

include(GoogleTest)
gtest_discover_tests(LiveVisionTests)