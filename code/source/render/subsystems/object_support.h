#pragma once
#include "def.h"
#include "../bin/shaders/shared_structures.h"
struct aiString;
struct aiScene;

/* Project namespace */
namespace gdr
{
  using gdr_object= long long;
  // Object system representation class
  class object_support
  {
    // this class will store indices in pools and will help work with all subsystems
    private:
      // pointer to Render
      render* Render;
      std::unordered_map<std::string, std::vector<gdr_object>> LoadedObjectTypes;

      // Load texture function
      int LoadTextureFromAssimp(aiString *path, aiScene* scene, std::string directory, bool isSrgb = false);
    public:
      // default constructor
      object_support(render *Rnd);

      // function which will import data and return gdr_object
      gdr_object CreateObject(const vertex* pVertex, size_t vertexCount, const UINT32* pIndices, size_t indexCount);

      // function which will import data and return gdr_object
      gdr_object DublicateObject(gdr_object original);

      // function which will import data and return gdr_object
      std::vector<gdr_object> CreateObjectsFromFile(std::string fileName);

      // function which will return transfroms by object index
      ObjectTransform &GetTransforms(gdr_object object);

      // function which will return material by object index
      ObjectMaterial &GetMaterial(gdr_object object);

      // default de-structor
      ~object_support();

      // CPU Pool with indices
      std::vector<ObjectIndices> CPUPool;
  };
}