#pragma once

#include <string>

#include "impeller/renderer/command_buffer.h"
#include "impeller/renderer/context.h"

namespace example {

class ExampleBase {
 public:
  struct Info {
    std::string name;
    std::string description;
  };

  virtual ~ExampleBase();

  virtual Info GetInfo() = 0;
  virtual bool Setup(impeller::Context& context) = 0;
  virtual bool Render(impeller::Context& context,
                      const impeller::RenderTarget& render_target,
                      impeller::CommandBuffer& command_buffer) = 0;
};

}  // namespace example
