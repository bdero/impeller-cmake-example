# Copyright 2013 The Flutter Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

cmake_minimum_required(VERSION 3.21)
if(APPLE)
    project(example LANGUAGES C CXX OBJC OBJCXX)
else()
    project(example LANGUAGES C CXX)
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(PROJECT_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(SOURCE_DIR ${PROJECT_DIR}/src)
set(THIRD_PARTY_DIR ${PROJECT_DIR}/third_party)
set(GENERATED_DIR ${PROJECT_DIR}/generated)
set(ASSETS_SOURCE_DIR ${PROJECT_DIR}/assets_src)
set(ASSETS_OUT_DIR ${PROJECT_DIR}/assets)
set(FLATBUFFERS_INCLUDE_DIR ${THIRD_PARTY_DIR}/impeller-cmake/third_party/flatbuffers/include)

if(CMAKE_BUILD_TYPE MATCHES Debug)
    # It's very important that IMPELLER_DEBUG is enabled whenever Impeller's
    # libraries are built with it, as some data structures in headers change
    # with the presence of this flag.
    add_compile_definitions(IMPELLER_DEBUG=1)
endif()

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
elseif(UNIX)
    set(CXX_STANDARD_LIBRARY libc++)
endif()

# Dependency: impeller-cmake
add_subdirectory(third_party/impeller-cmake)

# Dependency: GLFW
option(GLFW_BUILD_EXAMPLES "" OFF)
option(GLFW_BUILD_TESTS "" OFF)
option(GLFW_BUILD_DOCS "" OFF)
option(GLFW_INSTALL "" OFF)
if(APPLE)
elseif(UNIX)
    set(CMAKE_THREAD_LIBS_INIT "-lpthread")
endif()
add_subdirectory(third_party/glfw)

# Dependency: STB
add_library(stb STATIC "third_party/stb_image_impl.cc")

# Dependency: Dear ImGui
include("imgui.cmake")

# Asset importer binary
add_subdirectory(third_party/assimp)
add_subdirectory(src/importer)
target_compile_options(assimp PRIVATE "-Wno-unused-function")

# Shaders
add_gles_shader_library(
    NAME example
    SHADERS
        ${FLUTTER_ENGINE_DIR}/impeller/fixtures/impeller.vert
        ${FLUTTER_ENGINE_DIR}/impeller/fixtures/impeller.frag
        ${SOURCE_DIR}/examples/mesh/mesh_example.vert
        ${SOURCE_DIR}/examples/mesh/mesh_example.frag
    OUTPUT_DIR ${GENERATED_DIR}/shaders)
add_library(example_shader_lib STATIC
    "${GENERATED_DIR}/shaders/gles/example_shaders_gles.cc"
    "${GENERATED_DIR}/shaders/impeller.vert.cc"
    "${GENERATED_DIR}/shaders/impeller.frag.cc"
    "${GENERATED_DIR}/shaders/mesh_example.vert.cc"
    "${GENERATED_DIR}/shaders/mesh_example.frag.cc")
target_link_libraries(example_shader_lib PUBLIC impeller_renderer impeller_entity impeller_aiks)

# Examples binary
add_subdirectory(src/examples)
add_executable(example
    "src/main_gles.cc"
)
target_link_libraries(example PRIVATE glfw imgui examples)

# Import assets
import_model(
    TARGET example
    INPUTS ${ASSETS_SOURCE_DIR}/flutter_logo.fbx
    OUTPUT_DIR ${ASSETS_OUT_DIR})

import_texture(
    TARGET example
    INPUTS
        ${ASSETS_SOURCE_DIR}/flutter_logo_BaseColor.png
        ${ASSETS_SOURCE_DIR}/flutter_logo_Normal.png
        ${ASSETS_SOURCE_DIR}/flutter_logo_OcclusionRoughnessMetallic.png
    OUTPUT_DIR ${ASSETS_OUT_DIR})
