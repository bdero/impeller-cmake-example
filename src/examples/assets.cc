// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "assets.h"

#include <array>
#include <iostream>
#include <memory>

#include "impeller/core/allocator.h"
#include "impeller/core/formats.h"
#include "impeller/core/texture_descriptor.h"
#include "stb/stb_image.h"

namespace example {

struct RawImage {
  unsigned char* data;
  int width;
  int height;
  int channels;
};

RawImage LoadRGBA(const char* filename) {
  int width, height, channels;
  auto pixels = stbi_load(filename, &width, &height, &channels, STBI_rgb_alpha);
  if (!pixels) {
    std::cerr << "Failed to decode image file: " << filename << std::endl;
    return {};
  }
  if (channels != 4) {
    std::cerr << "Image has " << channels
              << " channels; changed to RGBA: " << filename << std::endl;
  }

  return {pixels, width, height, 4};
}

std::shared_ptr<impeller::Texture> LoadTexture(
    std::filesystem::path filename,
    impeller::Allocator& allocator,
    impeller::TextureUsageMask usage) {
  auto filename_str = filename.generic_string();
  auto raw_image = LoadRGBA(filename_str.c_str());
  if (!raw_image.data) {
    return nullptr;
  }

  impeller::TextureDescriptor texture_desc;
  texture_desc.type = impeller::TextureType::kTexture2D;
  texture_desc.format = impeller::PixelFormat::kR8G8B8A8UNormInt;
  texture_desc.size = {raw_image.width, raw_image.height};
  texture_desc.usage = usage;
  texture_desc.storage_mode = impeller::StorageMode::kHostVisible;

  auto texture = allocator.CreateTexture(texture_desc);
  if (!texture) {
    std::cerr << "Failed to allocate device texture for image: " << filename
              << std::endl;
    return nullptr;
  }
  texture->SetLabel(filename_str);

  if (!texture->SetContents(raw_image.data, raw_image.width * raw_image.height *
                                                raw_image.channels)) {
    std::cerr << "Failed to upload texture to device memory for image: "
              << filename << std::endl;
    return nullptr;
  }
  return texture;
}

std::shared_ptr<impeller::Texture> LoadTextureCube(
    std::array<std::filesystem::path, 6> filenames,
    impeller::Allocator& allocator,
    impeller::TextureUsageMask usage) {
  std::array<RawImage, 6> raw_images;
  for (size_t i = 0; i < filenames.size(); i++) {
    auto image = LoadRGBA(filenames[i].generic_string().c_str());
    if (!image.data) {
      return nullptr;
    }
    raw_images[i] = image;
  }

  impeller::TextureDescriptor texture_desc;
  texture_desc.type = impeller::TextureType::kTextureCube;
  texture_desc.format = impeller::PixelFormat::kR8G8B8A8UNormInt;
  texture_desc.size = {raw_images[0].width, raw_images[0].height};
  texture_desc.usage = usage;
  texture_desc.storage_mode = impeller::StorageMode::kHostVisible;

  auto texture = allocator.CreateTexture(texture_desc);
  if (!texture) {
    std::cerr << "Failed to allocate device texture for image: " << filenames[0]
              << std::endl;
    return nullptr;
  }
  texture->SetLabel(filenames[0].generic_string());

  for (size_t i = 0; i < filenames.size(); i++) {
    auto uploaded = texture->SetContents(
        raw_images[i].data,
        raw_images[i].width * raw_images[i].height * raw_images[i].channels, i);
    if (!uploaded) {
      std::cerr << "Failed to upload texture to device memory for image: "
                << filenames[0] << std::endl;
    }
  }

  return texture;
}

}  // namespace example
