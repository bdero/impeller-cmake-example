#include <array>
#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <memory>

#include "backends/imgui_impl_glfw.h"
#include "fml/mapping.h"
#include "imgui.h"
#include "impeller/playground/imgui/gles/imgui_shaders_gles.h"
#include "impeller/playground/imgui/imgui_impl_impeller.h"
#include "impeller/renderer/allocator.h"
#include "impeller/renderer/backend/gles/context_gles.h"
#include "impeller/renderer/backend/gles/reactor_gles.h"
#include "impeller/renderer/backend/gles/surface_gles.h"
#include "impeller/renderer/formats.h"
#include "impeller/renderer/renderer.h"
#include "impeller/renderer/texture_descriptor.h"

#define GLFW_INCLUDE_NONE
#include "third_party/glfw/include/GLFW/glfw3.h"

#include "examples/assets.h"
#include "examples/clock.h"
#include "examples/example_base.h"
#include "examples/mesh/mesh_example.h"
#include "examples/the_impeller/the_impeller_example.h"

#include "generated/shaders/gles/example_shaders_gles.h"
#include "generated/shaders/impeller.frag.h"
#include "generated/shaders/impeller.vert.h"

class ReactorWorker final : public impeller::ReactorGLES::Worker {
 public:
  ReactorWorker() = default;

  // |ReactorGLES::Worker|
  bool CanReactorReactOnCurrentThreadNow(
      const impeller::ReactorGLES& reactor) const override {
    impeller::ReaderLock lock(mutex_);
    auto found = reactions_allowed_.find(std::this_thread::get_id());
    if (found == reactions_allowed_.end()) {
      return false;
    }
    return found->second;
  }

  void SetReactionsAllowedOnCurrentThread(bool allowed) {
    impeller::WriterLock lock(mutex_);
    reactions_allowed_[std::this_thread::get_id()] = allowed;
  }

 private:
  mutable impeller::RWMutex mutex_;
  std::map<std::thread::id, bool> reactions_allowed_ IPLR_GUARDED_BY(mutex_);

  FML_DISALLOW_COPY_AND_ASSIGN(ReactorWorker);
};

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

#ifdef __APPLE__
  // ES Profiles are not supported on Mac.
  ::glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
#else   // __APPLE__
  ::glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
  ::glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
  ::glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
#endif  // __APPLE__
  //::glfwWindowHint(GLFW_SRGB_CAPABLE, GLFW_TRUE);
  ::glfwWindowHint(GLFW_RED_BITS, 8);
  ::glfwWindowHint(GLFW_GREEN_BITS, 8);
  ::glfwWindowHint(GLFW_BLUE_BITS, 8);
  ::glfwWindowHint(GLFW_ALPHA_BITS, 8);
  ::glfwWindowHint(GLFW_DEPTH_BITS, 24);
  ::glfwWindowHint(GLFW_STENCIL_BITS, 8);  // 8 bit stencil buffer
  ::glfwWindowHint(GLFW_SAMPLES, 4);       // 4xMSAA

  auto window =
      ::glfwCreateWindow(1024, 768, "Impeller example", nullptr, nullptr);
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

  auto context = impeller::ContextGLES::Create(
      std::move(gl), {std::make_shared<fml::NonOwnedMapping>(
                          impeller_imgui_shaders_gles_data,
                          impeller_imgui_shaders_gles_length),
                      std::make_shared<fml::NonOwnedMapping>(
                          impeller_example_shaders_gles_data,
                          impeller_example_shaders_gles_length)});
  if (!context) {
    std::cerr << "Failed to create Impeller context.";
    return EXIT_FAILURE;
  }

  auto worker = std::make_shared<ReactorWorker>();
  worker->SetReactionsAllowedOnCurrentThread(true);
  auto worker_id = context->AddReactorWorker(worker);
  if (!worker_id.has_value()) {
    std::cerr << "Failed to register GLES reactor worker.";
    return EXIT_FAILURE;
  }

  auto renderer = std::make_unique<impeller::Renderer>(context);

  //----------------------------------------------------------------------------
  /// Setup ImGui.
  ///

  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  fml::ScopedCleanupClosure destroy_imgui_context(
      []() { ImGui::DestroyContext(); });
  ImGui::StyleColorsDark();
  ImGui::GetIO().IniFilename = nullptr;

  ::ImGui_ImplGlfw_InitForOther(window, true);
  ::ImGui_ImplImpeller_Init(context);

  //----------------------------------------------------------------------------
  /// Setup examples.
  ///

  std::vector<std::unique_ptr<example::ExampleBase>> examples;
  examples.push_back(std::make_unique<example::TheImpellerExample>());
  examples.push_back(std::make_unique<example::MeshExample>());

  std::vector<const char*> example_names;

  for (auto& example : examples) {
    auto info = example->GetInfo();
    example_names.push_back(info.name);

    if (!example->Setup(*renderer->GetContext())) {
      return EXIT_FAILURE;
    }
  }

  //----------------------------------------------------------------------------
  /// Render.
  ///

  while (!::glfwWindowShouldClose(window)) {
    ::ImGui_ImplGlfw_NewFrame();

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

    ImGui::SetNextWindowPos({10, 10});

    impeller::Renderer::RenderCallback render_callback =
        [&renderer, &examples,
         &example_names](impeller::RenderTarget& render_target) -> bool {
      static int selected_example_index = 1;
      auto example = examples[selected_example_index].get();
      auto example_info = example->GetInfo();

      ImGui::NewFrame();
      ImGui::Begin(example_info.name, nullptr,
                   ImGuiWindowFlags_AlwaysAutoResize);
      {
        if (ImGui::SmallButton("<")) {
          selected_example_index -= 1;
        }
        ImGui::SameLine(200);
        if (ImGui::SmallButton(">")) {
          selected_example_index += 1;
        }
        while (selected_example_index < 0) {
          selected_example_index += example_names.size();
        }
        while (selected_example_index >= example_names.size()) {
          selected_example_index -= example_names.size();
        }
        ImGui::ListBox("##", &selected_example_index, example_names.data(),
                       example_names.size());
        ImGui::TextWrapped("%s", example_info.description);
      }
      ImGui::End();
      ImGui::Render();

      auto buffer = renderer->GetContext()->CreateCommandBuffer();
      if (!buffer) {
        return false;
      }
      buffer->SetLabel("Command Buffer");

      /// Setup depth attachment.

      {
        impeller::TextureDescriptor depth_texture_desc;
        depth_texture_desc.type = impeller::TextureType::kTexture2D;
        depth_texture_desc.format = impeller::PixelFormat::kDefaultColor;
        depth_texture_desc.size = render_target.GetRenderTargetSize();
        depth_texture_desc.usage = static_cast<impeller::TextureUsageMask>(
            impeller::TextureUsage::kRenderTarget);
        depth_texture_desc.sample_count = impeller::SampleCount::kCount1;

        impeller::DepthAttachment depth;
        depth.load_action = impeller::LoadAction::kClear;
        depth.store_action = impeller::StoreAction::kDontCare;
        depth.clear_depth = 1.0;
        depth.texture =
            renderer->GetContext()->GetResourceAllocator()->CreateTexture(
                impeller::StorageMode::kDeviceTransient, depth_texture_desc);

        render_target.SetDepthAttachment(depth);
      }

      // Render the selected example.
      if (!example->Render(*renderer->GetContext(), render_target, *buffer)) {
        return false;
      }

      // Render ImGui overlay.
      {
        if (render_target.GetColorAttachments().empty()) {
          return false;
        }
        auto color0 = render_target.GetColorAttachments().find(0)->second;
        color0.load_action = impeller::LoadAction::kLoad;
        render_target.SetColorAttachment(color0, 0);
        auto pass = buffer->CreateRenderPass(render_target);
        if (!pass) {
          return false;
        }
        pass->SetLabel("ImGui Render Pass");

        ::ImGui_ImplImpeller_RenderDrawData(ImGui::GetDrawData(), *pass);

        if (!pass->EncodeCommands()) {
          return false;
        }
      }

      return buffer->SubmitCommands();
    };
    renderer->Render(std::move(surface), render_callback);

    ::glfwPollEvents();
  }

  ::ImGui_ImplImpeller_Shutdown();
  ::ImGui_ImplGlfw_Shutdown();

  ::glfwDestroyWindow(window);
  ::glfwTerminate();
  return EXIT_SUCCESS;
}
