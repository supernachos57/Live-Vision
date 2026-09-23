cmake_minimum_required(VERSION 3.20)
project(Live-Vision VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.18.0
)

if(MSVC)
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
endif()

FetchContent_MakeAvailable(googletest)

find_package(Torch REQUIRED)
find_package(OpenCV REQUIRED COMPONENTS core imgcodecs imgproc)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${TORCH_CXX_FLAGS}")

add_executable(LiveVision src/main.cpp)
target_link_libraries(LiveVision PRIVATE "${TORCH_LIBRARIES}" ${OpenCV_LIBS})

if(MSVC)
    file(GLOB TORCH_DLLS "${TORCH_INSTALL_PREFIX}/lib/*.dll")

    add_custom_command(TARGET LiveVision POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${TORCH_DLLS}
        $<TARGET_FILE_DIR:LiveVision>
        VERBATIM
    )
endif()

add_executable(LiveVisionTests tests/test_main.cpp)
target_link_libraries(LiveVisionTests PRIVATE GTest::gtest_main)

enable_testing()
include(GoogleTest)
gtest_discover_tests(LiveVisionTests)
