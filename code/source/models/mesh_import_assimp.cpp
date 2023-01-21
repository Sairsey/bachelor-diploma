#include "p_header.h"
#ifdef min
#undef min
#endif
#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"
#ifndef min
#define min(a,b)            (((a) < (b)) ? (a) : (b))
#endif

struct mesh_assimp_importer
{
  std::string FileName;
  std::string ImportDirectory;
  gdr::mesh_import_data Result;
  // mapping for bones
  std::unordered_map<aiNode *, gdr_index> tmpBoneMapping;
  // mapping from assimp material indices to ours
  std::unordered_map<unsigned int, gdr_index> tmpMaterialMapping;
  
  //assimp specific data
  Assimp::Importer importer;
  const aiScene* scene;

  gdr_index ImportTreeFirstPass(aiNode *node, gdr_index ParentIndex = NONE_INDEX);
  void ImportTreeSecondPass(aiNode* node, gdr_index CurrentIndex = 0);
  gdr_index ImportTreeMesh(aiMesh* mesh, gdr_index ParentIndex);
  gdr_index GetTextureFromAssimp(aiMaterial* assimpMaterial, aiTextureType textureType);


  void Import();
};

gdr_index mesh_assimp_importer::ImportTreeFirstPass(aiNode* node, gdr_index ParentIndex)
{
  // first -> add new node in Result;
  Result.HierarchyNodes.emplace_back();
  gdr_index Current = Result.HierarchyNodes.size() - 1;

  // second -> fill all its fields;
  Result.HierarchyNodes[Current].Name = node->mName.C_Str();
  Result.HierarchyNodes[Current].Type = gdr_hier_node_type::node;
  Result.HierarchyNodes[Current].BoneOffset = mth::matr4f::Identity();
  Result.HierarchyNodes[Current].LocalTransform = mth::matr4f(node->mTransformation.a1, node->mTransformation.b1, node->mTransformation.c1, node->mTransformation.d1,
    node->mTransformation.a2, node->mTransformation.b2, node->mTransformation.c2, node->mTransformation.d2,
    node->mTransformation.a3, node->mTransformation.b3, node->mTransformation.c3, node->mTransformation.d3,
    node->mTransformation.a4, node->mTransformation.b4, node->mTransformation.c4, node->mTransformation.d4);

  tmpBoneMapping[node] = Current;

  Result.HierarchyNodes[Current].ParentIndex = ParentIndex;
  Result.HierarchyNodes[Current].ChildIndex = NONE_INDEX;
  Result.HierarchyNodes[Current].NextIndex = NONE_INDEX;
  for (unsigned int i = 0; i < node->mNumChildren; i++)
  {
    gdr_index Child = ImportTreeFirstPass(node->mChildren[i], Current);
    Result.HierarchyNodes[Child].NextIndex = Result.HierarchyNodes[Current].ChildIndex;
    Result.HierarchyNodes[Current].ChildIndex = Child;
  }

  return Current;
}

void mesh_assimp_importer::ImportTreeSecondPass(aiNode* node, gdr_index CurrentIndex)
{
  // import meshes
  for (int i = 0; i < node->mNumMeshes; i++)
  {
    gdr_index Mesh = ImportTreeMesh(scene->mMeshes[node->mMeshes[i]], CurrentIndex);
    if (Mesh != NONE_INDEX)
    {
      if (Result.HierarchyNodes[CurrentIndex].ChildIndex == NONE_INDEX)
        Result.HierarchyNodes[CurrentIndex].ChildIndex = Mesh;
      else 
      {
        gdr_index *Place = &Result.HierarchyNodes[Result.HierarchyNodes[CurrentIndex].ChildIndex].NextIndex;
        while (*Place != NONE_INDEX)
          Place = &Result.HierarchyNodes[*Place].NextIndex;
        *Place = Mesh;
      }
    }
  }

  // into hierarchy
  gdr_index ChildIndex = Result.HierarchyNodes[CurrentIndex].ChildIndex;
  for (int i = node->mNumChildren - 1; i >=0  ; i--)
  {
    ImportTreeSecondPass(node->mChildren[i], ChildIndex);
    ChildIndex = Result.HierarchyNodes[ChildIndex].NextIndex;
  }
}

gdr_index mesh_assimp_importer::GetTextureFromAssimp(aiMaterial* assimpMaterial, aiTextureType textureType)
{
  aiString Path;
  if (assimpMaterial->GetTexture(textureType, 0, &Path) != aiReturn_SUCCESS)
    return NONE_INDEX;

  int textureNumber = atoi(Path.C_Str() + 1);
  std::string fullpath;
  if (Path.C_Str()[0] == '*')
  {
    fullpath = ImportDirectory + scene->mTextures[textureNumber]->mFilename.C_Str() + (Path.C_Str() + 1) + "." + scene->mTextures[textureNumber]->achFormatHint;

    DWORD dwAttrib = GetFileAttributesA(fullpath.c_str());
    if (dwAttrib != INVALID_FILE_ATTRIBUTES && !(dwAttrib & FILE_ATTRIBUTE_DIRECTORY))
    {}
    else
    {
      FILE* F;
      fopen_s(&F, fullpath.c_str(), "wb");
      fwrite(scene->mTextures[textureNumber]->pcData, 1, scene->mTextures[textureNumber]->mWidth, F);
      fclose(F);
    }
  }
  else
  {
    fullpath = ImportDirectory + Path.C_Str();
    FILE* F;
    if (fopen_s(&F, fullpath.c_str(), "r") != 0)
      return NONE_INDEX;
    fclose(F);
  }
  
  gdr_index TextureIndex = Result.TexturesPaths.size();
  for (gdr_index i = 0; i < Result.TexturesPaths.size() && TextureIndex == Result.TexturesPaths.size(); i++)
    if (Result.TexturesPaths[i] == fullpath)
      TextureIndex = i;
  
  if (TextureIndex == Result.TexturesPaths.size())
    Result.TexturesPaths.push_back(fullpath);

  return TextureIndex;
}

gdr_index mesh_assimp_importer::ImportTreeMesh(aiMesh* mesh, gdr_index ParentIndex)
{
  // first -> add new node in Result;
  Result.HierarchyNodes.emplace_back();
  gdr_index Current = Result.HierarchyNodes.size() - 1;

  // second -> fill all its fields;
  Result.HierarchyNodes[Current].Name = mesh->mName.C_Str();
  Result.HierarchyNodes[Current].Type = gdr_hier_node_type::mesh;
  Result.HierarchyNodes[Current].ParentIndex = ParentIndex;
  Result.HierarchyNodes[Current].NextIndex = NONE_INDEX;
  Result.HierarchyNodes[Current].ChildIndex = NONE_INDEX;
  
  Result.HierarchyNodes[Current].vertices.reserve(mesh->mNumVertices);
  Result.HierarchyNodes[Current].indices.reserve(mesh->mNumFaces * 3);
  bool isHasBones = mesh->HasBones();

  // load indices
  for (int j = 0; j < (int)mesh->mNumFaces; j++)
  {
    if (mesh->mFaces[j].mNumIndices != 3)
    {
      continue;
    }
    Result.HierarchyNodes[Current].indices.push_back(mesh->mFaces[j].mIndices[0]);
    Result.HierarchyNodes[Current].indices.push_back(mesh->mFaces[j].mIndices[1]);
    Result.HierarchyNodes[Current].indices.push_back(mesh->mFaces[j].mIndices[2]);
  }

  if (Result.HierarchyNodes[Current].indices.size() == 0)
  {
    Result.HierarchyNodes.pop_back();
    return NONE_INDEX;
  }

  if (!mesh->HasTextureCoords(0))
  {
    Result.HierarchyNodes.pop_back();
    return NONE_INDEX;
  }

  // load vertices
  for (int j = 0; j < (int)mesh->mNumVertices; j++)
  {
    GDRVertex V;
    V.Pos = mth::vec3f({ mesh->mVertices[j].x, mesh->mVertices[j].y, mesh->mVertices[j].z });
    V.Normal = mth::vec3f({ mesh->mNormals[j].x, mesh->mNormals[j].y, mesh->mNormals[j].z });
    V.Normal.Normalize();
    aiVector3D uv;

    if (mesh->mTextureCoords[0])
      uv = mesh->mTextureCoords[0][j];

    V.UV = mth::vec2f({ uv.x, uv.y });

    V.Tangent = mth::vec3f{ mesh->mTangents[j].x, mesh->mTangents[j].y, mesh->mTangents[j].z };
    V.Tangent.Normalize();

    V.BonesIndices = mth::vec4<UINT>(NONE_INDEX, NONE_INDEX, NONE_INDEX, NONE_INDEX);
    V.BonesWeights = mth::vec4f(0);

    Result.HierarchyNodes[Current].vertices.push_back(V);
  }

  // fix bones in verices
  for (int i = 0; i < (int)mesh->mNumBones; i++)
  {
    gdr_index ourIndex = tmpBoneMapping[mesh->mBones[i]->mNode];
    Result.HierarchyNodes[ourIndex].BoneOffset = mth::matr4f(
      mesh->mBones[i]->mOffsetMatrix.a1, mesh->mBones[i]->mOffsetMatrix.b1, mesh->mBones[i]->mOffsetMatrix.c1, mesh->mBones[i]->mOffsetMatrix.d1,
      mesh->mBones[i]->mOffsetMatrix.a2, mesh->mBones[i]->mOffsetMatrix.b2, mesh->mBones[i]->mOffsetMatrix.c2, mesh->mBones[i]->mOffsetMatrix.d2,
      mesh->mBones[i]->mOffsetMatrix.a3, mesh->mBones[i]->mOffsetMatrix.b3, mesh->mBones[i]->mOffsetMatrix.c3, mesh->mBones[i]->mOffsetMatrix.d3,
      mesh->mBones[i]->mOffsetMatrix.a4, mesh->mBones[i]->mOffsetMatrix.b4, mesh->mBones[i]->mOffsetMatrix.c4, mesh->mBones[i]->mOffsetMatrix.d4);
    for (int j = 0; j < (int)mesh->mBones[i]->mNumWeights; j++)
    {
      int vertex_id = mesh->mBones[i]->mWeights[j].mVertexId;
      float weight = mesh->mBones[i]->mWeights[j].mWeight;
      if (weight == 0)
        continue;
      if (Result.HierarchyNodes[Current].vertices[vertex_id].BonesIndices.X == -1)
      {
        Result.HierarchyNodes[Current].vertices[vertex_id].BonesIndices.X = ourIndex;
        Result.HierarchyNodes[Current].vertices[vertex_id].BonesWeights.X = weight;
      }
      else if (Result.HierarchyNodes[Current].vertices[vertex_id].BonesIndices.Y == -1)
      {
        Result.HierarchyNodes[Current].vertices[vertex_id].BonesIndices.Y = ourIndex;
        Result.HierarchyNodes[Current].vertices[vertex_id].BonesWeights.Y = weight;
      }
      else if (Result.HierarchyNodes[Current].vertices[vertex_id].BonesIndices.Z == -1)
      {
        Result.HierarchyNodes[Current].vertices[vertex_id].BonesIndices.Z = ourIndex;
        Result.HierarchyNodes[Current].vertices[vertex_id].BonesWeights.Z = weight;
      }
      else if (Result.HierarchyNodes[Current].vertices[vertex_id].BonesIndices.W == -1)
      {
        Result.HierarchyNodes[Current].vertices[vertex_id].BonesIndices.W = ourIndex;
        Result.HierarchyNodes[Current].vertices[vertex_id].BonesWeights.W = weight;
      }
      else
      {
        OutputDebugString(L"ERROR! Number of Bone vertex index are greater than 4!!!\n");
      }
    }
  }

  // load Materials
  if (tmpMaterialMapping.find(mesh->mMaterialIndex) != tmpMaterialMapping.end())
    Result.HierarchyNodes[Current].MaterialIndex = tmpMaterialMapping[mesh->mMaterialIndex];
  else
  {
    Result.Materials.emplace_back();
    Result.HierarchyNodes[Current].MaterialIndex = Result.Materials.size() - 1;
    GDRGPUMaterial &newMaterial = Result.Materials[Result.HierarchyNodes[Current].MaterialIndex];
    aiMaterial *assimpMaterial = scene->mMaterials[mesh->mMaterialIndex];
    
    aiShadingMode shadingModel = (aiShadingMode)0x0;
    ai_real metallicFactor;
    ai_real glossFactor;
    ai_real shiness;
    aiString str;
    aiColor3D color(0.f, 0.f, 0.f);

    assimpMaterial->Get(AI_MATKEY_SHADING_MODEL, shadingModel);

    if (shadingModel == aiShadingMode_PBR_BRDF && (assimpMaterial->Get(AI_MATKEY_METALLIC_FACTOR, metallicFactor) == aiReturn_SUCCESS))
    {
      newMaterial.ShadeType = MATERIAL_SHADER_COOKTORRANCE_METALNESS;
      printf("ERROR");
    }
    else if (shadingModel == aiShadingMode_PBR_BRDF && (assimpMaterial->Get(AI_MATKEY_GLOSSINESS_FACTOR, glossFactor) == aiReturn_SUCCESS))
    {
      newMaterial.ShadeType = MATERIAL_SHADER_COOKTORRANCE_SPECULAR;
      printf("ERROR");
    }
    else if (shadingModel == aiShadingMode_Phong || shadingModel == aiShadingMode_Blinn)
    {
      newMaterial.ShadeType = MATERIAL_SHADER_PHONG;
      
      // Shiness 
      assimpMaterial->Get(AI_MATKEY_SHININESS, shiness);
      GDRGPUMaterialPhongGetShiness(newMaterial) = shiness / 4.0;
      assimpMaterial->Get(AI_MATKEY_SHININESS_STRENGTH, shiness);
      GDRGPUMaterialPhongGetShiness(newMaterial) *= shiness;

      // Ka
      {
        GDRGPUMaterialPhongGetDiffuseMapIndex(newMaterial) = GetTextureFromAssimp(assimpMaterial, aiTextureType_AMBIENT);
        assimpMaterial->Get(AI_MATKEY_COLOR_AMBIENT, color);
        // HACK
        if (GDRGPUMaterialColorGetColorMapIndex(newMaterial) != NONE_INDEX && color == aiColor3D(0.f))
          color = aiColor3D(1.f);
        GDRGPUMaterialPhongGetAmbient(newMaterial) = mth::vec3f(color.r, color.g, color.b);
      }

      // Kd
      {
        GDRGPUMaterialPhongGetDiffuseMapIndex(newMaterial) = GetTextureFromAssimp(assimpMaterial, aiTextureType_DIFFUSE);
        assimpMaterial->Get(AI_MATKEY_COLOR_DIFFUSE, color);
        // HACK
        if (GDRGPUMaterialPhongGetDiffuseMapIndex(newMaterial) != NONE_INDEX && color == aiColor3D(0.f))
          color = aiColor3D(1.f);
        GDRGPUMaterialPhongGetDiffuse(newMaterial) = mth::vec3f(color.r, color.g, color.b);
      }

      // Ks
      {
        GDRGPUMaterialPhongGetDiffuseMapIndex(newMaterial) = GetTextureFromAssimp(assimpMaterial, aiTextureType_SPECULAR);
        assimpMaterial->Get(AI_MATKEY_COLOR_SPECULAR, color);
        // HACK
        if (GDRGPUMaterialPhongGetSpecularMapIndex(newMaterial) != NONE_INDEX && color == aiColor3D(0.f))
          color = aiColor3D(1.f);
        GDRGPUMaterialPhongGetSpecular(newMaterial) = mth::vec3f(color.r, color.g, color.b);
      }

      // Normal map
      GDRGPUMaterialPhongGetNormalMapIndex(newMaterial) = GetTextureFromAssimp(assimpMaterial, aiTextureType_NORMALS);
    }
    else
    {
      newMaterial.ShadeType = MATERIAL_SHADER_COLOR;
      assimpMaterial->Get(AI_MATKEY_COLOR_DIFFUSE, color);
      GDRGPUMaterialColorGetColorMapIndex(newMaterial) = GetTextureFromAssimp(assimpMaterial, aiTextureType_DIFFUSE);
      
      // HACK
      if (GDRGPUMaterialColorGetColorMapIndex(newMaterial) != NONE_INDEX && color == aiColor3D(0.f))
        color = aiColor3D(1.f);
      
      GDRGPUMaterialColorGetColor(newMaterial) = mth::vec3f(color.r, color.g, color.b);
    }
  }

  return Current;
}

void mesh_assimp_importer::Import()
{
  scene = importer.ReadFile(FileName,
    aiProcessPreset_TargetRealtime_Fast |
    aiProcess_SplitLargeMeshes |
    aiProcess_FlipUVs |
    aiProcess_PopulateArmatureData);
  
  if (!scene)
    return;
  
  Result.FileName = FileName;
  Result.RootTransform.Transform = mth::matr::Identity();
  Result.RootTransform.minAABB = mth::vec3f(0, 0, 0);
  Result.RootTransform.maxAABB = mth::vec3f(0, 0, 0);
  
  ImportTreeFirstPass(scene->mRootNode);
  ImportTreeSecondPass(scene->mRootNode);

  // calculate min-max AABB

  importer.FreeScene();
}


gdr::mesh_import_data gdr::ImportMeshAssimp(std::string filename)
{
  mesh_assimp_importer myImporter;

  myImporter.FileName = filename;

  size_t last_slash_idx = filename.rfind('\\');
  if (std::string::npos != last_slash_idx)
  {
    myImporter.ImportDirectory = filename.substr(0, last_slash_idx);
  }
  else
  {
    size_t last_slash_idx = filename.rfind('/');
    if (std::string::npos != last_slash_idx)
    {
      myImporter.ImportDirectory = filename.substr(0, last_slash_idx) + "/";
    }
  }

  myImporter.Import();

  return myImporter.Result;
}