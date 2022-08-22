#include "mesh_example.h"

#include <array>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>

#include "imgui/imgui.h"
#include "impeller/geometry/matrix.h"
#include "impeller/geometry/scalar.h"
#include "impeller/renderer/allocator.h"
#include "impeller/renderer/buffer_view.h"
#include "impeller/renderer/command.h"
#include "impeller/renderer/formats.h"
#include "impeller/renderer/pipeline_library.h"
#include "impeller/renderer/render_pass.h"
#include "impeller/renderer/render_target.h"
#include "impeller/renderer/sampler_library.h"
#include "impeller/renderer/vertex_buffer.h"
#include "impeller/tessellator/tessellator.h"

#include "examples/assets.h"

#include "generated/importer/mesh_flatbuffers.h"
#include "generated/shaders/mesh_example.frag.h"
#include "generated/shaders/mesh_example.vert.h"

namespace example {

ExampleBase::Info MeshExample::GetInfo() {
  return {
      .name = "Mesh Example",
      .description =
          "An FBX with texture coordinates and normals/tangents imported ahead "
          "of time. The importer tool is located under `src/importer`.",
  };
}

bool MeshExample::Setup(impeller::Context& context) {
  //----------------------------------------------------------------------------
  /// Load/unpack the model.
  ///

  std::ifstream in;
  const char* filename = "assets/flutter_logo.model";
  in.open(filename, std::ios::binary | std::ios::in);
  in.seekg(0, std::ios::end);
  auto size = in.tellg();
  in.seekg(0, std::ios::beg);

  std::vector<char> data(size);
  in.read(data.data(), size);
  in.close();

  if (!in) {
    std::cerr << "Failed to read file: " << filename << std::endl;
    return false;
  }

  fb::MeshT mesh;
  fb::GetMesh(data.data())->UnPackTo(&mesh);

  //----------------------------------------------------------------------------
  /// Create sampler and load textures.
  ///

  impeller::SamplerDescriptor sampler_desc;
  sampler_desc.min_filter = impeller::MinMagFilter::kLinear;
  sampler_desc.mag_filter = impeller::MinMagFilter::kLinear;
  sampler_ = context.GetSamplerLibrary()->GetSampler(sampler_desc);

  const auto asset_path = std::filesystem::current_path() / "assets/";

  base_color_texture_ =
      example::LoadTexture(asset_path / "flutter_logo_BaseColor.png",
                           *context.GetResourceAllocator());
  if (!base_color_texture_) {
    std::cerr << "Failed to load base color texture." << std::endl;
  }

  normal_texture_ = example::LoadTexture(asset_path / "flutter_logo_Normal.png",
                                         *context.GetResourceAllocator());
  if (!normal_texture_) {
    std::cerr << "Failed to load normal texture." << std::endl;
  }

  occlusion_roughness_metallic_texture_ = example::LoadTexture(
      asset_path / "flutter_logo_OcclusionRoughnessMetallic.png",
      *context.GetResourceAllocator());
  if (!occlusion_roughness_metallic_texture_) {
    std::cerr << "Failed to load occlusion/roughness/metallic texture."
              << std::endl;
  }

  //----------------------------------------------------------------------------
  /// Upload vertices/indices to the device.
  ///

  auto vertices_size = sizeof(fb::Vertex) * mesh.vertices.size();
  auto indices_size = sizeof(uint16_t) * mesh.indices.size();
  auto device_buffer = context.GetResourceAllocator()->CreateBuffer(
      impeller::StorageMode::kHostVisible, vertices_size + indices_size);

  if (!device_buffer) {
    std::cerr << "Failed to create device buffer." << std::endl;
    return false;
  }

  if (!device_buffer->CopyHostBuffer(
          reinterpret_cast<uint8_t*>(mesh.vertices.data()),
          impeller::Range{0, vertices_size}, 0)) {
    std::cerr << "Failed to upload vertices to device buffer." << std::endl;
    return false;
  }

  if (!device_buffer->CopyHostBuffer(
          reinterpret_cast<uint8_t*>(mesh.indices.data()),
          impeller::Range{0, indices_size}, vertices_size)) {
    std::cerr << "Failed to upload indices to device buffer." << std::endl;
    return false;
  }

  vertex_buffer_ = {
      .vertex_buffer =
          impeller::BufferView{.buffer = device_buffer,
                               .range = impeller::Range{0, vertices_size}},
      .index_buffer =
          impeller::BufferView{
              .buffer = device_buffer,
              .range = impeller::Range{vertices_size, indices_size}},
      .index_count = mesh.indices.size(),
      .index_type = impeller::IndexType::k16bit,
  };

  //----------------------------------------------------------------------------
  /// Build the pipeline.
  ///

  auto pipeline_desc =
      impeller::PipelineBuilder<VS, FS>::MakeDefaultPipelineDescriptor(context);
  pipeline_desc->SetSampleCount(impeller::SampleCount::kCount4);
  pipeline_desc->SetWindingOrder(impeller::WindingOrder::kCounterClockwise);
  pipeline_desc->SetCullMode(impeller::CullMode::kBackFace);
  pipeline_desc->SetDepthStencilAttachmentDescriptor({
      .depth_compare = impeller::CompareFunction::kLess,
      .depth_write_enabled = true,
  });
  pipeline_ =
      context.GetPipelineLibrary()->GetRenderPipeline(pipeline_desc).get();
  if (!pipeline_ || !pipeline_->IsValid()) {
    std::cerr << "Failed to initialize pipeline for mesh example.";
    return false;
  }

  return true;
}

bool MeshExample::Render(impeller::Context& context,
                         const impeller::RenderTarget& render_target,
                         impeller::CommandBuffer& command_buffer) {
  clock_.Tick();

  static float exposure = 5;
  ImGui::SliderFloat("Exposure", &exposure, 0, 15);

  auto pass = command_buffer.CreateRenderPass(render_target);
  if (!pass) {
    return false;
  }

  impeller::Command cmd;
  cmd.label = "Mesh Example";
  cmd.pipeline = pipeline_;

  cmd.BindVertices(vertex_buffer_);

  auto time = clock_.GetTime();

  VS::VertInfo vs_uniform;
  vs_uniform.mvp =
      impeller::Matrix::MakePerspective(impeller::Degrees{60},
                                        pass->GetRenderTargetSize(), 10, 100) *
      impeller::Matrix::MakeTranslation({0, 0, -50}) *
      impeller::Matrix::MakeScale({0.3, 0.3, 0.3}) *
      impeller::Matrix::MakeRotationY(impeller::Radians{-0.4f * time}) *
      impeller::Matrix::MakeRotationZ(
          impeller::Radians{std::sin(time * 0.43f) / 5}) *
      impeller::Matrix::MakeRotationX(
          impeller::Radians{std::cos(time * 0.27f) / 4});
  VS::BindVertInfo(cmd, pass->GetTransientsBuffer().EmplaceUniform(vs_uniform));

  FS::FragInfo fs_uniform;
  fs_uniform.exposure = exposure;
  fs_uniform.camera_position = {0, 0, -50};
  FS::BindFragInfo(cmd, pass->GetTransientsBuffer().EmplaceUniform(fs_uniform));

  FS::BindBaseColorTexture(cmd, base_color_texture_, sampler_);
  FS::BindNormalTexture(cmd, normal_texture_, sampler_);
  FS::BindOcclusionRoughnessMetallicTexture(
      cmd, occlusion_roughness_metallic_texture_, sampler_);

  if (!pass->AddCommand(std::move(cmd))) {
    return false;
  }

  if (!pass->EncodeCommands()) {
    return false;
  }

  return true;
}

}  // namespace example
