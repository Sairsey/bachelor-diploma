#pragma once
#include "def.h"
#include "../bin/shaders/shared_structures.h"
struct aiString;
struct aiScene;
struct aiNode;
struct aiMesh;

/* Project namespace */
namespace gdr
{
  // different node types
  enum struct gdr_node_type
  {
    node = 0,  // node of hierarchy. May be bone in case of skinning
    mesh,      // mesh
    light,     // if we have light attached to some point
    total_amount_of_types
  };

  using gdr_index = long long;

  // node representation class
  struct gdr_node
  {
    private:
      render *Render;
    public:
      std::string Name;
      gdr_index ParentIndex;          // index of Parent node in NodesPool
      gdr_index Index;                // index of this node in NodesPool
      std::vector<gdr_index> Childs;  // Child nodes

      // We have additional parameter for skinning 
      // #define OBJECT_PARAMETER_SKINNED     0x2
      // if it is not set, then we set index of parent node transform here and in CPUPool.
      // if it is set, then we set index of FileNode transform, and use AABB`s calculated for whole model
      // Skinning now not supported so for time being we use only first approach
      gdr_index TransformIndex; // index of this node transform in transforms pool
      gdr_node_type NodeType;   // type of node

      // Transforms for hierarchic computation
      mth::matr4f LocalTransform;
      std::vector<std::pair<float, mth::vec3f>> AnimationKeyFramePositions;
      std::vector<std::pair<float, mth::vec4f>> AnimationKeyFrameRotations;
      std::vector< std::pair<float, mth::vec3f>> AnimationKeyFrameScales;
      float Duration = -1;
      mth::matr4f GlobalTransform;
      bool IsTransformCalculated; // True if we need recalculation of Transforms

      // Mesh specific fields
      gdr_index MeshIndex; // index of this node transform in object_support CPUPool

      gdr_node(render *Rnd) : Render(Rnd) {};

      ObjectTransform GetTransform();
      mth::matr4f &GetTransformEditable();
      ObjectMaterial &GetMaterial();
  };

  // Object system representation class
  class object_support
  {
    // this class will store indices in pools and will help work with all subsystems
    private:
      // pointer to Render
      render* Render;

      // used while parsing
      std::string directory;

      std::unordered_map<std::string, gdr_index> LoadedFiles;

      std::unordered_map<aiNode *, gdr_index> tmpBoneMapping;

      std::queue<gdr_index> NodesToRecalc;

      // Load texture function
      int LoadTextureFromAssimp(aiString *path, aiScene* scene, std::string directory, bool isSrgb = false);

      // Load assimp tree recursive function
      gdr_index LoadAssimpTreeFirstPass(const aiScene* scene, aiNode* node, gdr_index ParentNode);

      // Load assimp tree recursive function
      gdr_index LoadAssimpTreeSecondPass(const aiScene* scene, aiNode *node, gdr_index ParentNode, gdr_index CurrentNode);

      // Load node from assimp tree recursive function
      gdr_index LoadAssimpTreeMesh(const aiScene* scene, aiMesh* mesh, gdr_index ParentNode);

      // Update Node transforms recurcive
      void UpdateNode(gdr_index nodeIndex);

    public:
      // Pool with all nodes
      std::vector<gdr_node> NodesPool;

      // default constructor
      object_support(render *Rnd);

      // function which will import data and return index for gdr_node
      gdr_index CreateObject(const vertex* pVertex, size_t vertexCount, const UINT32* pIndices, size_t indexCount);

      // function which will import data and return gdr_object
      gdr_index DublicateObject(gdr_index original, gdr_index parent = -1);

      // function which will import data and return index for gdr_node
      gdr_index CreateObjectFromFile(std::string fileName);

      // Recompute Translations and AABBs
      void UpdateAllNodes(void);

      void MarkNodeToRecalc(gdr_index nodeIndex);

      void SetAnimationTime(gdr_index nodeIndex, float time, float offset = 0, float duration = -1);

      // default de-structor
      ~object_support();

      // CPU Pool with indices
      std::vector<ObjectIndices> CPUPool;
  };
}