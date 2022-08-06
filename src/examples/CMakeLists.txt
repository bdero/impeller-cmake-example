add_library(examples STATIC 
    "assets.cc"
    "clock.cc"
    "example_base.cc"
    "the_impeller_example.cc"
    "${GENERATED_DIR}/shaders/gles/example_shaders_gles.c"
    "${GENERATED_DIR}/shaders/impeller.vert.cc"
    "${GENERATED_DIR}/shaders/impeller.frag.cc"
)

target_link_libraries(examples PRIVATE impeller_renderer stb)
target_include_directories(examples PRIVATE
    ${PROJECT_DIR}
    ${SOURCE_DIR}
    ${THIRD_PARTY_DIR}
    ${FLUTTER_DIR})
