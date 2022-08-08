add_executable(importer
    "importer_main.cc"
)

flatbuffers_schema(
    TARGET importer
    INPUT ${SOURCE_DIR}/importer/mesh.fbs
    OUTPUT_DIR ${GENERATED_DIR}/importer)

target_link_libraries(importer PRIVATE assimp)
target_include_directories(importer PRIVATE ${THIRD_PARTY_DIR}/assimp/include)
target_include_directories(importer PUBLIC
    ${PROJECT_DIR}
    ${SOURCE_DIR}
    ${THIRD_PARTY_DIR}/impeller-cmake/third_party/flatbuffers/include)

# import_model(
#    TARGET dependent
#    INPUT filename
#    OUTPUT_DIR path
# )
function(import_model)
    cmake_parse_arguments(ARG "" "TARGET;INPUT;OUTPUT_DIR" "" ${ARGN})

    get_filename_component(INPUT_FILENAME ${ARG_INPUT} NAME_WE)

    set(OUTPUT_FILE "${ARG_OUTPUT_DIR}/${INPUT_FILENAME}.model")
    add_custom_command(
        TARGET "${ARG_TARGET}"
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "${ARG_OUTPUT_DIR}"
        COMMAND "$<TARGET_FILE:importer>"
            "${ARG_INPUT}"
            "${OUTPUT_FILE}"
        BYPRODUCTS "${OUTPUT_FILE}"
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
        COMMENT "Importing model ${ARG_INPUT}")
endfunction()
