#include <array>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "assimp/Importer.hpp"
#include "assimp/postprocess.h"
#include "assimp/scene.h"

#include "flatbuffers/flatbuffer_builder.h"
#include "generated/importer/mesh_flatbuffers.h"

int main(int argc, char* argv[]) {
  if (argc != 3) {
    std::cerr << "Expected 2 arguments, but " << argc - 1 << " provided."
              << std::endl;
    std::cerr << "Usage: " << argv[0] << " INPUT_FILE OUTPUT_FILE" << std::endl;
    return 2;  // Invalid argument.
  }
  std::string input_file = argv[1];
  std::string output_file = argv[2];

  //----------------------------------------------------------------------------
  /// Parse the source model.
  ///

  Assimp::Importer importer;

  unsigned int postprocess_flags =
      aiProcess_CalcTangentSpace | aiProcess_Triangulate |
      aiProcess_JoinIdenticalVertices | aiProcess_SortByPType;
  auto scene = importer.ReadFile(input_file, postprocess_flags);
  if (scene == nullptr) {
    std::cerr << "Failed to load scene: " << importer.GetErrorString()
              << std::endl;
    return EXIT_FAILURE;
  }

  example::fb::MeshT mesh;


  //----------------------------------------------------------------------------
  /// Build a flatbuffer with the model data.
  ///

  flatbuffers::FlatBufferBuilder builder;
  builder.Finish(example::fb::Mesh::Pack(builder, &mesh));

  //----------------------------------------------------------------------------
  /// Write the flatbuffer to disk.
  ///
  {
    auto ptr = builder.GetBufferPointer();
    auto size = builder.GetSize();
    std::ofstream out(output_file,
                      std::ios::binary | std::ios::out | std::ios::trunc);

    out.write(reinterpret_cast<char*>(ptr), size);
    out.close();

    if (!out) {
      std::cerr << "Failed to write file: " << output_file << std::endl;
      return EXIT_FAILURE;
    }
  }
  builder.Release();

  return EXIT_SUCCESS;
}
