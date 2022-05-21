#include <iostream>
#include <memory>

#include "impeller/geometry/size.h"
#include "impeller/renderer/backend/gles/context_gles.h"
#include "impeller/renderer/backend/gles/proc_table_gles.h"
#include "impeller/renderer/backend/gles/surface_gles.h"
#include "impeller/renderer/formats.h"
#include "impeller/renderer/render_target.h"
#include "impeller/renderer/renderer.h"

#define GLFW_INCLUDE_NONE
#include "third_party/glfw/include/GLFW/glfw3.h"

int main() {
  //----------------------------------------------------------------------------
  /// Create a GLFW window.
  ///

  if (::glfwInit() != GLFW_TRUE) {
    std::cerr << "Couldn't initialize GLFW." << std::endl;
    return EXIT_FAILURE;
  }
  ::glfwSetErrorCallback([](int code, const char* description) {
    std::cerr << "GLFW Error '" << description << "'  (" << code << ").";
  });

  ::glfwDefaultWindowHints();

  ::glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
  ::glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
  ::glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

  ::glfwWindowHint(GLFW_RED_BITS, 8);
  ::glfwWindowHint(GLFW_GREEN_BITS, 8);
  ::glfwWindowHint(GLFW_BLUE_BITS, 8);
  ::glfwWindowHint(GLFW_ALPHA_BITS, 8);
  ::glfwWindowHint(GLFW_DEPTH_BITS, 0);    // no depth buffer
  ::glfwWindowHint(GLFW_STENCIL_BITS, 8);  // 8 bit stencil buffer
  ::glfwWindowHint(GLFW_SAMPLES, 4);       // 4xMSAA

  auto window =
      ::glfwCreateWindow(800, 600, "Impeller example", nullptr, nullptr);
  if (!window) {
    std::cerr << "Couldn't create GLFW window.";
    ::glfwTerminate();
    return EXIT_FAILURE;
  }
  ::glfwMakeContextCurrent(window);

  //----------------------------------------------------------------------------
  /// Create an Impeller context.
  ///

  auto resolver = [](const char* name) -> void* {
    return reinterpret_cast<void*>(::glfwGetProcAddress(name));
  };
  auto gl = std::make_unique<impeller::ProcTableGLES>(resolver);
  if (!gl->IsValid()) {
    std::cerr << "Failed to create a valid GLES proc table.";
    return EXIT_FAILURE;
  }

  auto context = impeller::ContextGLES::Create(std::move(gl), {});
  auto renderer = std::make_unique<impeller::Renderer>(context);

  //----------------------------------------------------------------------------
  /// Render.
  ///

  while (!::glfwWindowShouldClose(window)) {
    /// Get the next surface.

    impeller::SurfaceGLES::SwapCallback swap_callback = [window]() -> bool {
      ::glfwSwapBuffers(window);
      return true;
    };
    int width, height;
    ::glfwGetFramebufferSize(window, &width, &height);
    auto surface = impeller::SurfaceGLES::WrapFBO(
        context, swap_callback, 0u, impeller::PixelFormat::kB8G8R8A8UNormInt,
        impeller::ISize::MakeWH(width, height));

    /// Render to the surface.

    impeller::Renderer::RenderCallback render_callback =
        [&renderer](impeller::RenderTarget& render_target) -> bool {
      return true;
    };
    renderer->Render(std::move(surface), render_callback);

    ::glfwPollEvents();
  }

  ::glfwDestroyWindow(window);
  ::glfwTerminate();
  return EXIT_SUCCESS;
}
