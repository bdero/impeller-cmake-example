# Copyright 2013 The Flutter Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# This toolchain definition expects the environment variable
# FLUTTER_ENGINE_SRC_DIR to be the absolute path to a full Flutter engine
# checkout.
#
# Optionally, the enviornment variable FLUTTER_GOMA_DIR may be set to the
# absolute path to the `gomacc` binary. Googlers can use this to do a Goma
# accelerated build.
#
# For the Goma accelerated build to work locally (that is, outside of the
# CI environment), the environment variable FLUTTER_OSX_SYSROOT should be set
# to the path that results from the script:
#
# $ flutter/engine/src/build/mac/find_sdk.py --print_sdk_path 10.15 --symlink xcode-sysroot
#
# From a Flutter engine checkout, and `xcode-sysroot` is the desired location
# of the symlink that the script creates. Set FLUTTER_OSX_SYSROOT to
# the absolute path to `xcode-sysroot`.

set(FLUTTER_ENGINE_SRC_DIR "$ENV{FLUTTER_ENGINE_SRC_DIR}")
if(NOT FLUTTER_ENGINE_SRC_DIR OR NOT IS_DIRECTORY "${FLUTTER_ENGINE_SRC_DIR}")
    message(SEND_ERROR
        "Unable to configure any targets because the Flutter Engine directory "
        "(FLUTTER_ENGINE_SRC_DIR) couldn't be found: "
        "    '${FLUTTER_ENGINE_SRC_DIR}'")
    return()
endif()

set(FLUTTER_ENGINE_SRC_DIR "$ENV{FLUTTER_ENGINE_SRC_DIR}")
set(FLUTTER_ENGINE_DIR "${FLUTTER_ENGINE_SRC_DIR}/flutter")

set(TOOLCHAIN_BIN_DIR "${FLUTTER_ENGINE_SRC_DIR}/buildtools/mac-x64/clang/bin")
set(CMAKE_C_COMPILER "${TOOLCHAIN_BIN_DIR}/clang")
set(CMAKE_CXX_COMPILER "${TOOLCHAIN_BIN_DIR}/clang++")
set(CMAKE_CXX_LINKER "${TOOLCHAIN_BIN_DIR}/clang++")
set(CMAKE_AR "${TOOLCHAIN_BIN_DIR}/llvm-ar")
add_compile_options("-Wno-deprecated-builtins")

set(FLUTTER_GOMA_DIR "$ENV{FLUTTER_GOMA_DIR}")
if(FLUTTER_GOMA_DIR)
    set(CMAKE_C_COMPILER_LAUNCHER "${FLUTTER_GOMA_DIR}/gomacc")
    set(CMAKE_CXX_COMPILER_LAUNCHER "${FLUTTER_GOMA_DIR}/gomacc")
endif()

set(FLUTTER_OSX_SYSROOT "$ENV{FLUTTER_OSX_SYSROOT}")
if(FLUTTER_OSX_SYSROOT)
    set(CMAKE_OSX_SYSROOT "${FLUTTER_OSX_SYSROOT}")
    set(CMAKE_OSX_DEPLOYMENT_TARGET "10.15")
    set(CMAKE_MACOSX_RPATH "1")

    add_compile_options("-nostdinc++")
    include_directories("${CMAKE_OSX_SYSROOT}/usr/include/c++/v1")
    add_link_options("-L${CMAKE_OSX_SYSROOT}/usr/lib")
endif()
