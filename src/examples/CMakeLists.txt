add_library(examples STATIC
    "assets.cc"
    "clock.cc"
    "example_base.cc"
    "mesh_example.cc"
    "the_impeller_example.cc"
)

target_link_libraries(examples PRIVATE impeller_renderer stb)
target_link_libraries(examples PUBLIC example_shader_lib)
target_include_directories(examples PUBLIC
    ${PROJECT_DIR}
    ${SOURCE_DIR}
    ${THIRD_PARTY_DIR}
    ${FLATBUFFERS_INCLUDE_DIR})
