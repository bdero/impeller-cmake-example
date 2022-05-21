#include <iostream>

#define GLFW_INCLUDE_NONE
#include "third_party/glfw/include/GLFW/glfw3.h"

#include "impeller/renderer/backend/gles/context_gles.h"
#include "impeller/renderer/backend/gles/surface_gles.h"

int main() {
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

  while (!::glfwWindowShouldClose(window)) {
    ::glfwPollEvents();
  }

  ::glfwDestroyWindow(window);
  return EXIT_SUCCESS;
}
