add_library(examples STATIC 
    "assets.cc"
    "clock.cc")

target_link_libraries(examples PRIVATE impeller_renderer stb)
target_include_directories(examples PRIVATE
    ${PROJECT_DIR}
    ${THIRD_PARTY_DIR}
    ${FLUTTER_DIR})
message(STATUS "THIRD PARTY DIR: ${THIRD_PARTY_DIR}")
