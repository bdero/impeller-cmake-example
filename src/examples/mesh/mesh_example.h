#pragma once

#include "examples/example_base.h"

#include <memory>

#include "impeller/core/sampler.h"
#include "impeller/core/vertex_buffer.h"
#include "impeller/renderer/pipeline.h"
#include "impeller/renderer/pipeline_descriptor.h"

#include "examples/clock.h"

#include "generated/shaders/mesh_example.frag.h"
#include "generated/shaders/mesh_example.vert.h"

namespace example {

class MeshExample final : public ExampleBase {
 public:
  using VS = impeller::MeshExampleVertexShader;
  using FS = impeller::MeshExampleFragmentShader;

  Info GetInfo() override;
  bool Setup(impeller::Context& context) override;
  bool Render(impeller::Context& context,
              const impeller::RenderTarget& render_target,
              impeller::CommandBuffer& command_buffer) override;

 private:
  example::Clock clock_;

  std::shared_ptr<impeller::Pipeline<impeller::PipelineDescriptor>> pipeline_;
  impeller::VertexBuffer vertex_buffer_;

  std::shared_ptr<const impeller::Sampler> sampler_;

  std::shared_ptr<impeller::Texture> base_color_texture_;
  std::shared_ptr<impeller::Texture> normal_texture_;
  std::shared_ptr<impeller::Texture> occlusion_roughness_metallic_texture_;
};

}  // namespace example
