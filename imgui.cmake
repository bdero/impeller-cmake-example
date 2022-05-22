impellerc(
  OPENGL_ES
  INPUT ${CMAKE_CURRENT_SOURCE_DIR}/third_party/impeller-cmake/third_party/flutter/impeller/playground/imgui/imgui_raster.frag
  SL ${GENERATED_DIR}/impeller/playground/imgui/imgui_raster.frag.glsl
  SPIRV ${GENERATED_DIR}/impeller/playground/imgui/imgui_raster.frag.spirv
  REFLECTION_HEADER ${GENERATED_DIR}/impeller/playground/imgui/imgui_raster.frag.h
  REFLECTION_CC ${GENERATED_DIR}/impeller/playground/imgui/imgui_raster.frag.cc)
impellerc(
  OPENGL_ES
  INPUT ${CMAKE_CURRENT_SOURCE_DIR}/third_party/impeller-cmake/third_party/flutter/impeller/playground/imgui/imgui_raster.vert
  SL ${GENERATED_DIR}/impeller/playground/imgui/imgui_raster.vert.glsl
  SPIRV ${GENERATED_DIR}/impeller/playground/imgui/imgui_raster.vert.spirv
  REFLECTION_HEADER ${GENERATED_DIR}/impeller/playground/imgui/imgui_raster.vert.h
  REFLECTION_CC ${GENERATED_DIR}/impeller/playground/imgui/imgui_raster.vert.cc)

set(IMGUI_SOURCE_FILES
    "third_party/imgui/imgui.cpp"
    "third_party/imgui/imgui_draw.cpp"
    "third_party/imgui/imgui_widgets.cpp"
    "third_party/imgui/imgui_tables.cpp"
    "third_party/imgui/imgui_demo.cpp"
    "third_party/imgui/backends/imgui_impl_glfw.cpp"
    "third_party/impeller-cmake/third_party/flutter/impeller/playground/imgui/imgui_impl_impeller.cc"
    "${GENERATED_DIR}/impeller/playground/imgui/imgui_raster.frag.cc"
    "${GENERATED_DIR}/impeller/playground/imgui/imgui_raster.vert.cc")

add_library(imgui STATIC ${IMGUI_SOURCE_FILES})

target_include_directories(imgui
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR} # For "third_party/*"
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party # For "imgui/*"
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party/imgui # For "imgui.h"
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party/impeller-cmake/third_party/flutter # For "impeller/*"
        ${GENERATED_DIR})
target_link_libraries(imgui PUBLIC glfw impeller_renderer)
