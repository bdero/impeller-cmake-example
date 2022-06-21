#include <cstdlib>
#include <iostream>
#include <memory>

#include "backends/imgui_impl_glfw.h"
#include "fml/mapping.h"
#include "imgui.h"
#include "impeller/base/thread.h"
#include "impeller/geometry/point.h"
#include "impeller/geometry/size.h"
#include "impeller/playground/imgui/gles/imgui_shaders_gles.h"
#include "impeller/playground/imgui/imgui_impl_impeller.h"
#include "impeller/renderer/backend/gles/context_gles.h"
#include "impeller/renderer/backend/gles/proc_table_gles.h"
#include "impeller/renderer/backend/gles/reactor_gles.h"
#include "impeller/renderer/backend/gles/surface_gles.h"
#include "impeller/renderer/command.h"
#include "impeller/renderer/formats.h"
#include "impeller/renderer/pipeline_builder.h"
#include "impeller/renderer/render_target.h"
#include "impeller/renderer/renderer.h"
#include "impeller/renderer/vertex_buffer_builder.h"

#define GLFW_INCLUDE_NONE
#include "third_party/glfw/include/GLFW/glfw3.h"

#include "shaders/gles/example_shaders_gles.h"
#include "shaders/impeller.frag.h"
#include "shaders/impeller.vert.h"

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

  ::glfwWindowHint(GLFW_RED_BITS, 8);
  ::glfwWindowHint(GLFW_GREEN_BITS, 8);
  ::glfwWindowHint(GLFW_BLUE_BITS, 8);
  ::glfwWindowHint(GLFW_ALPHA_BITS, 8);
  ::glfwWindowHint(GLFW_DEPTH_BITS, 0);    // no depth buffer
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
  /// Setup showcase.
  ///

  using VS = impeller::ImpellerVertexShader;
  using FS = impeller::ImpellerFragmentShader;

  auto pipeline_desc =
      impeller::PipelineBuilder<VS, FS>::MakeDefaultPipelineDescriptor(
          *renderer->GetContext());
  pipeline_desc->SetSampleCount(impeller::SampleCount::kCount4);
  auto pipeline = renderer->GetContext()
                      ->GetPipelineLibrary()
                      ->GetRenderPipeline(pipeline_desc)
                      .get();
  if (!pipeline || !pipeline->IsValid()) {
    std::cerr << "Failed to initialize pipeline for showcase.";
    return EXIT_FAILURE;
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

    impeller::Renderer::RenderCallback render_callback =
        [&renderer, &pipeline](impeller::RenderTarget& render_target) -> bool {
      ImGui::NewFrame();
      static bool demo = true;
      ImGui::ShowDemoWindow(&demo);
      ImGui::Render();

      auto buffer = renderer->GetContext()->CreateRenderCommandBuffer();
      if (!buffer) {
        return false;
      }
      buffer->SetLabel("Command Buffer");

      // Render Impeller showcase.
      {
        auto pass = buffer->CreateRenderPass(render_target);
        if (!pass) {
          return false;
        }

        impeller::Command cmd;
        cmd.label = "Impeller SDF showcase";
        cmd.pipeline = pipeline;

        auto size = render_target.GetRenderTargetSize();

        impeller::VertexBufferBuilder<VS::PerVertexData> builder;
        builder.AddVertices({{impeller::Point()},
                             {impeller::Point(0, size.height)},
                             {impeller::Point(size.width, 0)},
                             {impeller::Point(size.width, 0)},
                             {impeller::Point(0, size.height)},
                             {impeller::Point(size.width, size.height)}});
        cmd.BindVertices(
            builder.CreateVertexBuffer(pass->GetTransientsBuffer()));

        VS::FrameInfo vs_uniform;
        vs_uniform.mvp = impeller::Matrix::MakeOrthographic(size);
        VS::BindFrameInfo(
            cmd, pass->GetTransientsBuffer().EmplaceUniform((vs_uniform)));

        FS::FragInfo fs_uniform;
        fs_uniform.texture_size = impeller::Point(size);
        fs_uniform.time = fml::TimePoint::Now().ToEpochDelta().ToSecondsF();
        FS::BindFragInfo(
            cmd, pass->GetTransientsBuffer().EmplaceUniform(fs_uniform));
        // FS::BindBlueNoise(cmd, blue_noise, noise_sampler);
        // FS::BindCubeMap(cmd, cube_map, cube_map_sampler);

        if (!pass->AddCommand(cmd)) {
          return false;
        }

        if (!pass->EncodeCommands(
                renderer->GetContext()->GetTransientsAllocator())) {
          return false;
        }
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

        if (!pass->EncodeCommands(
                renderer->GetContext()->GetTransientsAllocator())) {
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
