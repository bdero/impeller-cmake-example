#include "assets.h"

#include <array>
#include <iostream>
#include <memory>

#include "impeller/renderer/allocator.h"
#include "impeller/renderer/formats.h"
#include "impeller/renderer/texture_descriptor.h"
#include "stb/stb_image.h"

namespace example {

std::shared_ptr<impeller::Texture> LoadTexture(
    const char* filename,
    impeller::Allocator& allocator,
    impeller::TextureUsageMask usage) {
  int width, height, channels;
  auto pixels = stbi_load(filename, &width, &height, &channels, STBI_rgb_alpha);
  if (!pixels) {
    std::cerr << "Failed to decode image file: " << filename << std::endl;
    return nullptr;
  }
  if (channels != 4) {
    std::cerr << "Image must be RGBA to upload, but has " << channels
              << " channels: " << filename << std::endl;
    return nullptr;
  }

  impeller::TextureDescriptor texture_desc;
  texture_desc.type = impeller::TextureType::kTexture2D;
  texture_desc.format = impeller::PixelFormat::kR8G8B8A8UNormInt;
  texture_desc.size = {width, height};
  texture_desc.usage = usage;

  auto texture = allocator.CreateTexture(impeller::StorageMode::kHostVisible,
                                         texture_desc);
  if (!texture) {
    std::cerr << "Failed to allocate device texture for image: " << filename
              << std::endl;
    return nullptr;
  }
  texture->SetLabel(filename);

  if (!texture->SetContents(pixels, width * height * channels)) {
    std::cerr << "Failed to upload texture to device memory for image: "
              << filename << std::endl;
    return nullptr;
  }
  return texture;
}

}  // namespace example
