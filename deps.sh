#!/usr/bin/env bash
# Copyright 2013 The Flutter Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

set -e

echo "Fetching git submodules..."
git submodule update --init --recursive
echo "Done fetching git submodules."
echo

bash third_party/impeller-cmake/deps.sh
