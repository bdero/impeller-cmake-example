cmake_minimum_required(VERSION 3.21)
project(example)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(GENERATED_DIR ${CMAKE_CURRENT_BINARY_DIR}/generated)

if(WIN32)
    add_compile_definitions(
        WIN32_LEAN_AND_MEAN
        NOMINMAX
        UNICODE)
    # Use the static MSVC runtime library everywhere because:
    #   1. the dynamic runtime doesn't work with FML, and
    #   2. the same runtime needs to be used everywhere.
    # Set compile options directly instead of using the `MSVC_RUNTIME_LIBRARY`
    # CMake property because setting said property in the GLOBAL scope doesn't
    # work for some reason.
    add_compile_options(
        $<$<CONFIG:>:/MT>
        $<$<CONFIG:Debug>:/MTd>
        $<$<CONFIG:Release>:/MT>)
endif()

add_subdirectory(third_party/impeller-cmake)

option(GLFW_BUILD_EXAMPLES "" OFF)
option(GLFW_BUILD_TESTS "" OFF)
option(GLFW_BUILD_DOCS "" OFF)
option(GLFW_INSTALL "" OFF)
add_subdirectory(third_party/glfw)

include("imgui.cmake")

add_executable(example "main.cc")
target_link_libraries(example PRIVATE impeller_renderer glfw imgui)
