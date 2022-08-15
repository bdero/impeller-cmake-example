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

  // This setting combined with the aiProcess_Triangulate postprocess ensures
  // all faces contain exactly 3 indices.
  importer.SetPropertyInteger(AI_CONFIG_PP_SBP_REMOVE,
                              aiPrimitiveType_LINE | aiPrimitiveType_POINT);

  unsigned int postprocess_flags =
      aiProcess_CalcTangentSpace | aiProcess_Triangulate |
      aiProcess_JoinIdenticalVertices | aiProcess_SortByPType;
  auto scene = importer.ReadFile(input_file, postprocess_flags);
  if (scene == nullptr) {
    std::cerr << "Failed to load scene: " << importer.GetErrorString()
              << std::endl;
    return EXIT_FAILURE;
  }

  if (!scene->HasMeshes()) {
    std::cerr << "Can't import scene because it doesn't contain any meshes."
              << std::endl;
    return EXIT_FAILURE;
  }

  auto ai_mesh = scene->mMeshes[0];

  if (!ai_mesh->HasPositions()) {
    std::cerr << "Can't import scene because the mesh doens't contain any "
                 "vertex positions."
              << std::endl;
    return EXIT_FAILURE;
  }

  if (!ai_mesh->HasFaces()) {
    std::cerr
        << "Can't import scene because the mesh doens't contain any faces."
        << std::endl;
    return EXIT_FAILURE;
  }

  if (!ai_mesh->HasNormals()) {
    std::cerr << "Warning: Mesh has no normals! Filling with zeroes."
              << std::endl;
  }

  if (!ai_mesh->HasTextureCoords(0)) {
    std::cerr << "Warning: Mesh has no texture coorinates! Filling with zeroes."
              << std::endl;
  }

  //----------------------------------------------------------------------------
  /// Build a flatbuffer with the model data.
  ///

  example::fb::MeshT mesh;

  for (int vertex_i = 0; vertex_i < ai_mesh->mNumVertices; vertex_i++) {
    aiVector3D position = ai_mesh->mVertices[vertex_i];

    aiVector3D normal;
    if (ai_mesh->HasNormals()) {
      normal = ai_mesh->mNormals[vertex_i];
    }

    aiVector3D texture_coords(0, 0, 0);
    if (ai_mesh->HasTextureCoords(0)) {
      texture_coords = ai_mesh->mTextureCoords[0][vertex_i];
    }

    example::fb::Vertex vtx({position.x, position.y, position.z},
                            {normal.x, normal.y, normal.z},
                            {texture_coords.x, texture_coords.y});
    mesh.vertices.push_back(std::move(vtx));
  }

  for (int face_i = 0; face_i < ai_mesh->mNumFaces; face_i++) {
    auto face = ai_mesh->mFaces[face_i];
    if (face.mNumIndices != 3) {
      std::cerr << "Unable to parse face " << face_i
                << " of mesh 0 because it contains " << face.mNumIndices
                << " indices." << std::endl;
      continue;
    }

    for (int index_i = 0; index_i < face.mNumIndices; index_i++) {
      mesh.indices.push_back(face.mIndices[index_i]);
    }
  }

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
