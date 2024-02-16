// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include <memory>

#include "examples/clock.h"
#include "impeller/core/host_buffer.h"
#include "impeller/core/sampler.h"
#include "impeller/core/texture.h"
#include "impeller/renderer/pipeline.h"
#include "impeller/renderer/pipeline_descriptor.h"

#include "examples/example_base.h"

#include "generated/shaders/impeller.frag.h"
#include "generated/shaders/impeller.vert.h"

namespace example {

class TheImpellerExample final : public ExampleBase {
 public:
  using VS = impeller::ImpellerVertexShader;
  using FS = impeller::ImpellerFragmentShader;

  Info GetInfo() override;
  bool Setup(impeller::Context& context) override;
  bool Render(impeller::Context& context,
              const impeller::RenderTarget& render_target,
              impeller::CommandBuffer& command_buffer) override;

 private:
  example::Clock clock_;

  std::shared_ptr<impeller::HostBuffer> transients_buffer_;

  std::shared_ptr<impeller::Texture> blue_noise_texture_;

  std::shared_ptr<impeller::Texture> cube_map_texture_;

  std::shared_ptr<impeller::Pipeline<impeller::PipelineDescriptor>> pipeline_;
};

}  // namespace example
