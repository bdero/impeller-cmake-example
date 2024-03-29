// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include <filesystem>
#include <memory>

#include "impeller/core/allocator.h"
#include "impeller/core/formats.h"
#include "impeller/core/texture.h"

namespace example {

std::shared_ptr<impeller::Texture> LoadTexture(
    std::filesystem::path filename,
    impeller::Allocator& allocator,
    impeller::TextureUsageMask usage = static_cast<impeller::TextureUsageMask>(
        impeller::TextureUsage::kShaderRead));

std::shared_ptr<impeller::Texture> LoadTextureCube(
    std::array<std::filesystem::path, 6> filenames,
    impeller::Allocator& allocator,
    impeller::TextureUsageMask usage = static_cast<impeller::TextureUsageMask>(
        impeller::TextureUsage::kShaderRead));

}  // namespace example
