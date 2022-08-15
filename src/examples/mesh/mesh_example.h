#pragma once

#include "examples/example_base.h"

#include <memory>

#include "impeller/renderer/pipeline.h"
#include "impeller/renderer/vertex_buffer.h"

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

  std::shared_ptr<impeller::Pipeline> pipeline_;
  impeller::VertexBuffer vertex_buffer_;
};

}  // namespace example
