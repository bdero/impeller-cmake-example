cmake_minimum_required(VERSION 3.21)
if(APPLE)
    project(example LANGUAGES C CXX OBJC OBJCXX)
else()
    project(example LANGUAGES C CXX)
endif()

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

# Dependency: impeller-cmake
add_subdirectory(third_party/impeller-cmake)

# Dependency: GLFW
option(GLFW_BUILD_EXAMPLES "" OFF)
option(GLFW_BUILD_TESTS "" OFF)
option(GLFW_BUILD_DOCS "" OFF)
option(GLFW_INSTALL "" OFF)
add_subdirectory(third_party/glfw)

# Dependency: STB
add_library(stb STATIC "third_party/stb.cc")

# Dependency: Dear ImGui
include("imgui.cmake")

# Shaders
add_gles_shader_library(
    NAME example
    SHADERS
        ${FLUTTER_ENGINE_DIR}/impeller/fixtures/impeller.vert
        ${FLUTTER_ENGINE_DIR}/impeller/fixtures/impeller.frag
    OUTPUT_DIR ${GENERATED_DIR}/shaders)

add_executable(example
    "src/main_gles.cc"
    "src/assets.cc"
    "${GENERATED_DIR}/shaders/gles/example_shaders_gles.c"
    "${GENERATED_DIR}/shaders/impeller.vert.cc"
    "${GENERATED_DIR}/shaders/impeller.frag.cc")
target_link_libraries(example PRIVATE impeller_renderer glfw imgui stb)
