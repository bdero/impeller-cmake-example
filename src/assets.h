#include <filesystem>
#include <memory>

#include "impeller/renderer/allocator.h"
#include "impeller/renderer/formats.h"
#include "impeller/renderer/texture.h"

namespace example {

std::shared_ptr<impeller::Texture> LoadTexture(
    const char* filename,
    impeller::Allocator& allocator,
    impeller::TextureUsageMask usage = static_cast<impeller::TextureUsageMask>(
        impeller::TextureUsage::kShaderRead));

std::shared_ptr<impeller::Texture> LoadTextureCube(
    std::array<const char*, 6> filenames,
    impeller::Allocator& allocator,
    impeller::TextureUsageMask usage = static_cast<impeller::TextureUsageMask>(
        impeller::TextureUsage::kShaderRead));

}  // namespace example
