#!/usr/bin/env bash
set -e

echo "Fetching git submodules..."
git submodule update --init --recursive
echo "Done fetching git submodules."
echo

bash third_party/impeller-cmake/deps.sh
