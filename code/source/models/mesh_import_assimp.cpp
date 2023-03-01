#include "p_header.h"
#ifdef min
#undef min
#endif
#include "assimp/Importer.hpp"
#include "assimp/GltfMaterial.h"
#include "assimp/scene.h"
#include "assimp/postprocess.h"
#ifndef min
#define min(a,b)            (((a) < (b)) ? (a) : (b))
#endif
#include "stb_image.h"

struct mesh_assimp_importer
{
  std::string FileName;
  std::string ImportDirectory;
  gdr::model_import_data Result;
  std::vector<gdr::model_import_data> Results;

  // mapping for bones
  std::unordered_map<aiNode *, gdr_index> tmpBoneMapping;
  // mapping from assimp material indices to ours
  std::unordered_map<unsigned int, gdr_index> tmpMaterialMapping;
  
  //assimp specific data
  Assimp::Importer importer;
  const aiScene* scene;

  gdr_index ImportTreeFirstPass(aiNode *node, gdr_index ParentIndex = NONE_INDEX);
  void ImportTreeSecondPass(aiNode* node, gdr_index CurrentIndex = 0, mth::matr4f Offset = mth::matr4f::Identity());
  gdr_index ImportTreeMesh(aiMesh* mesh, gdr_index ParentIndex, mth::matr4f Offset = mth::matr4f::Identity());
  gdr_index GetTextureFromAssimp(aiMaterial* assimpMaterial, aiTextureType textureType);

  bool IsTextureTransparent(std::string path);

  void CalculateGlobalKeyframes(gdr_index MyIndex);
  void CalculateAABB(gdr_index MyIndex, gdr_index KeyframeIndex);


  void Import();
  void ImportSplitted();
};

gdr_index mesh_assimp_importer::ImportTreeFirstPass(aiNode* node, gdr_index ParentIndex)
{
  // first -> add new node in Result;
  Result.HierarchyNodes.emplace_back();
  gdr_index Current = (gdr_index)(Result.HierarchyNodes.size() - 1);

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
    if (node->mChildren[i]->mName == aiString("rigidbodies"))
      continue;
    gdr_index Child = ImportTreeFirstPass(node->mChildren[i], Current);
    Result.HierarchyNodes[Child].NextIndex = Result.HierarchyNodes[Current].ChildIndex;
    Result.HierarchyNodes[Current].ChildIndex = Child;
  }

  return Current;
}

void mesh_assimp_importer::CalculateAABB(gdr_index MyIndex, gdr_index KeyframeIndex)
{
  if (Result.HierarchyNodes[MyIndex].Type == gdr_hier_node_type::mesh)
  {
    for (auto& Vert : Result.HierarchyNodes[MyIndex].Vertices)
    {
      mth::vec3f pos;
      for (int i = 0; i < 4; i++)
      {
        if (Vert.BonesIndices[i] != NONE_INDEX)
        {
          gdr_index boneIndex = Result.HierarchyNodes[MyIndex].BonesMapping[Vert.BonesIndices[i]];
          mth::matr4f globalTransform = (KeyframeIndex == NONE_INDEX) ?
            Result.HierarchyNodes[boneIndex].GlobalTransform :
            mth::matr4f::BuildTransform(
              Result.HierarchyNodes[boneIndex].GlobalKeyframes[KeyframeIndex].scale,
              Result.HierarchyNodes[boneIndex].GlobalKeyframes[KeyframeIndex].rotationQuat,
              Result.HierarchyNodes[boneIndex].GlobalKeyframes[KeyframeIndex].pos);
          pos += (Result.HierarchyNodes[boneIndex].BoneOffset * globalTransform) * Vert.Pos * Vert.BonesWeights[i];
        }
      }

      for (int component_index = 0; component_index < 3; component_index++)
      {
        Result.RootTransform.maxAABB[component_index] = max(Result.RootTransform.maxAABB[component_index], pos[component_index]);
        Result.RootTransform.minAABB[component_index] = min(Result.RootTransform.minAABB[component_index], pos[component_index]);
      }
    }
  }

  // into hierarchy
  gdr_index ChildIndex = Result.HierarchyNodes[MyIndex].ChildIndex;
  
    
  while (ChildIndex != NONE_INDEX)
  {
    CalculateAABB(ChildIndex, KeyframeIndex);
    ChildIndex = Result.HierarchyNodes[ChildIndex].NextIndex;
  }
}

void mesh_assimp_importer::ImportTreeSecondPass(aiNode* node, gdr_index CurrentIndex, mth::matr4f Offset)
{
  // import meshes
  for (unsigned i = 0; i < node->mNumMeshes; i++)
  {
    gdr_index Mesh = ImportTreeMesh(scene->mMeshes[node->mMeshes[i]], CurrentIndex, Result.HierarchyNodes[CurrentIndex].LocalTransform * Offset);
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

  // copy GlobalTransform
  Result.HierarchyNodes[CurrentIndex].GlobalTransform = Result.HierarchyNodes[CurrentIndex].LocalTransform * Offset;

  // into hierarchy
  gdr_index ChildIndex = Result.HierarchyNodes[CurrentIndex].ChildIndex;
  for (int i = node->mNumChildren - 1; i >=0  ; i--)
  {
    if (node->mChildren[i]->mName == aiString("rigidbodies"))
      continue;
    ImportTreeSecondPass(node->mChildren[i], ChildIndex, Result.HierarchyNodes[CurrentIndex].LocalTransform * Offset);
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
    fullpath = ImportDirectory + "\\" + scene->mTextures[textureNumber]->mFilename.C_Str() + (Path.C_Str() + 1) + "." + scene->mTextures[textureNumber]->achFormatHint;

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
    fullpath = ImportDirectory + "\\" + Path.C_Str();
    FILE* F;
    if (fopen_s(&F, fullpath.c_str(), "r") != 0)
      return NONE_INDEX;
    fclose(F);
  }
  
  gdr_index TextureIndex = (gdr_index)(Result.TexturesPaths.size());
  for (gdr_index i = 0; i < Result.TexturesPaths.size() && TextureIndex == Result.TexturesPaths.size(); i++)
    if (Result.TexturesPaths[i] == fullpath)
      TextureIndex = i;
  
  if (TextureIndex == Result.TexturesPaths.size())
    Result.TexturesPaths.push_back(fullpath);

  return TextureIndex;
}

gdr_index mesh_assimp_importer::ImportTreeMesh(aiMesh* mesh, gdr_index ParentIndex, mth::matr4f Offset)
{
  // first -> add new node in Result;
  Result.HierarchyNodes.emplace_back();
  gdr_index Current = (gdr_index)(Result.HierarchyNodes.size() - 1);

  // second -> fill all its fields;
  Result.HierarchyNodes[Current].Name = mesh->mName.C_Str();
  Result.HierarchyNodes[Current].Type = gdr_hier_node_type::mesh;
  Result.HierarchyNodes[Current].ParentIndex = ParentIndex;
  Result.HierarchyNodes[Current].NextIndex = NONE_INDEX;
  Result.HierarchyNodes[Current].ChildIndex = NONE_INDEX;

  Result.HierarchyNodes[Current].Vertices.reserve(mesh->mNumVertices);
  Result.HierarchyNodes[Current].Indices.reserve(mesh->mNumFaces * 3);
  bool isHasBones = mesh->HasBones();

  // load indices
  for (int j = 0; j < (int)mesh->mNumFaces; j++)
  {
    if (mesh->mFaces[j].mNumIndices != 3)
    {
      continue;
    }
    Result.HierarchyNodes[Current].Indices.push_back(mesh->mFaces[j].mIndices[0]);
    Result.HierarchyNodes[Current].Indices.push_back(mesh->mFaces[j].mIndices[1]);
    Result.HierarchyNodes[Current].Indices.push_back(mesh->mFaces[j].mIndices[2]);
  }

  if (Result.HierarchyNodes[Current].Indices.size() == 0)
  {
    Result.HierarchyNodes.pop_back();
    return NONE_INDEX;
  }

  /*
  if (!mesh->HasTextureCoords(0))
  {
    Result.HierarchyNodes.pop_back();
    return NONE_INDEX;
  }
  */

  mth::vec3f minAABB = mth::vec3f(INFINITY);
  mth::vec3f maxAABB = mth::vec3f(-INFINITY);
  // load vertices
  for (int j = 0; j < (int)mesh->mNumVertices; j++)
  {
    GDRVertex V;
    V.Pos = mth::vec3f({ mesh->mVertices[j].x, mesh->mVertices[j].y, mesh->mVertices[j].z });
    V.Normal = mth::vec3f({ mesh->mNormals[j].x, mesh->mNormals[j].y, mesh->mNormals[j].z });
    V.Normal.Normalize();
    aiVector3D uv;

    if (mesh->mTextureCoords[0])
    {
        uv = mesh->mTextureCoords[0][j];

        V.UV = mth::vec2f({ uv.x, uv.y });

        V.Tangent = mth::vec3f{ mesh->mTangents[j].x, mesh->mTangents[j].y, mesh->mTangents[j].z };
    }
    else
    {
        V.UV = { 0, 0 };
        V.Tangent = { 0, 0, 1 };
    }
    V.Tangent.Normalize();

    V.BonesIndices = mth::vec4<UINT>(NONE_INDEX, NONE_INDEX, NONE_INDEX, NONE_INDEX);
    V.BonesWeights = mth::vec4f(0,0,0,0);

    Result.HierarchyNodes[Current].Vertices.push_back(V);
  }

  // fix bones in vertices
  for (int i = 0; i < (int)mesh->mNumBones; i++)
  {
    gdr_index nodeIndex = tmpBoneMapping[mesh->mBones[i]->mNode];

    Result.HierarchyNodes[nodeIndex].BoneOffset = mth::matr4f(
      mesh->mBones[i]->mOffsetMatrix.a1, mesh->mBones[i]->mOffsetMatrix.b1, mesh->mBones[i]->mOffsetMatrix.c1, mesh->mBones[i]->mOffsetMatrix.d1,
      mesh->mBones[i]->mOffsetMatrix.a2, mesh->mBones[i]->mOffsetMatrix.b2, mesh->mBones[i]->mOffsetMatrix.c2, mesh->mBones[i]->mOffsetMatrix.d2,
      mesh->mBones[i]->mOffsetMatrix.a3, mesh->mBones[i]->mOffsetMatrix.b3, mesh->mBones[i]->mOffsetMatrix.c3, mesh->mBones[i]->mOffsetMatrix.d3,
      mesh->mBones[i]->mOffsetMatrix.a4, mesh->mBones[i]->mOffsetMatrix.b4, mesh->mBones[i]->mOffsetMatrix.c4, mesh->mBones[i]->mOffsetMatrix.d4);

    gdr_index nodeBoneMapping = NONE_INDEX;
    for (int j = 0; j < Result.HierarchyNodes[Current].BonesMapping.size() && nodeBoneMapping == NONE_INDEX; j++)
      if (Result.HierarchyNodes[Current].BonesMapping[j] == nodeIndex)
        nodeBoneMapping = NONE_INDEX;

    if (nodeBoneMapping == NONE_INDEX)
    {
      nodeBoneMapping = Result.HierarchyNodes[Current].BonesMapping.size();
      Result.HierarchyNodes[Current].BonesMapping.push_back(nodeIndex);
    }

    for (int j = 0; j < (int)mesh->mBones[i]->mNumWeights; j++)
    {
      int vertex_id = mesh->mBones[i]->mWeights[j].mVertexId;
      float weight = mesh->mBones[i]->mWeights[j].mWeight;
      if (weight == 0)
        continue;
      if (Result.HierarchyNodes[Current].Vertices[vertex_id].BonesIndices.X == NONE_INDEX)
      {
        Result.HierarchyNodes[Current].Vertices[vertex_id].BonesIndices.X = nodeBoneMapping;
        Result.HierarchyNodes[Current].Vertices[vertex_id].BonesWeights.X = weight;
      }
      else if (Result.HierarchyNodes[Current].Vertices[vertex_id].BonesIndices.Y == NONE_INDEX)
      {
        Result.HierarchyNodes[Current].Vertices[vertex_id].BonesIndices.Y = nodeBoneMapping;
        Result.HierarchyNodes[Current].Vertices[vertex_id].BonesWeights.Y = weight;
      }
      else if (Result.HierarchyNodes[Current].Vertices[vertex_id].BonesIndices.Z == NONE_INDEX)
      {
        Result.HierarchyNodes[Current].Vertices[vertex_id].BonesIndices.Z = nodeBoneMapping;
        Result.HierarchyNodes[Current].Vertices[vertex_id].BonesWeights.Z = weight;
      }
      else if (Result.HierarchyNodes[Current].Vertices[vertex_id].BonesIndices.W == NONE_INDEX)
      {
        Result.HierarchyNodes[Current].Vertices[vertex_id].BonesIndices.W = nodeBoneMapping;
        Result.HierarchyNodes[Current].Vertices[vertex_id].BonesWeights.W = weight;
      }
      else
      {
        GDR_FAILED("ERROR! Number of Bone vertex index are greater than 4!!!\n");
      }
    }
  }

  if (Result.HierarchyNodes[Current].BonesMapping.size() == 0 && ParentIndex != NONE_INDEX)
  {
    Result.HierarchyNodes[Current].BonesMapping.push_back(ParentIndex);
    for (int j = 0; j < (int)mesh->mNumVertices; j++)
    {
       Result.HierarchyNodes[Current].Vertices[j].BonesIndices.X = 0;
       Result.HierarchyNodes[Current].Vertices[j].BonesWeights.X = 1.0;
    }
  }

  // load Materials
  if (tmpMaterialMapping.find(mesh->mMaterialIndex) != tmpMaterialMapping.end())
    Result.HierarchyNodes[Current].MaterialIndex = tmpMaterialMapping[mesh->mMaterialIndex];
  else
  {
    Result.Materials.emplace_back();
    Result.HierarchyNodes[Current].MaterialIndex = (gdr_index)(Result.Materials.size() - 1);
    GDRGPUMaterial &newMaterial = Result.Materials[Result.HierarchyNodes[Current].MaterialIndex];
    aiMaterial *assimpMaterial = scene->mMaterials[mesh->mMaterialIndex];
    
    aiShadingMode shadingModel = (aiShadingMode)0x0;
    ai_real metallicFactor;
    ai_real glossFactor;
    ai_real shiness;
    aiString str;
    aiColor3D color(0.f, 0.f, 0.f);

    assimpMaterial->Get(AI_MATKEY_SHADING_MODEL, shadingModel);

    if (shadingModel == aiShadingMode_PBR_BRDF && (assimpMaterial->Get(AI_MATKEY_GLOSSINESS_FACTOR, glossFactor) == aiReturn_SUCCESS))
    {
        newMaterial.ShadeType = MATERIAL_SHADER_COOKTORRANCE_SPECULAR;
        // Ambient Occlusion
        GDRGPUMaterialCookTorranceGetAmbientOcclusionMapIndex(newMaterial) = GetTextureFromAssimp(assimpMaterial, aiTextureType_LIGHTMAP);

        // Albedo
        GDRGPUMaterialCookTorranceGetAlbedoMapIndex(newMaterial) = GetTextureFromAssimp(assimpMaterial, aiTextureType_DIFFUSE);
        assimpMaterial->Get(AI_MATKEY_COLOR_DIFFUSE, color);
        // HACK
        if (GDRGPUMaterialCookTorranceGetAlbedoMapIndex(newMaterial) != NONE_INDEX && color == aiColor3D(0.f))
            color = aiColor3D(1.f);
        GDRGPUMaterialCookTorranceGetAlbedo(newMaterial) = mth::vec3f(color.r, color.g, color.b);

        // Glossiness
        if (assimpMaterial->Get(AI_MATKEY_GLOSSINESS_FACTOR, glossFactor) == aiReturn_SUCCESS)
            GDRGPUMaterialCookTorranceGetGlossiness(newMaterial) = glossFactor;
        else
            GDRGPUMaterialCookTorranceGetGlossiness(newMaterial) = 0.1f;

        // Specular
        if (assimpMaterial->Get(AI_MATKEY_COLOR_SPECULAR, color) == aiReturn_SUCCESS)
            GDRGPUMaterialCookTorranceGetSpecular(newMaterial) = mth::vec3f(color.r, color.g, color.b);
        else
            GDRGPUMaterialCookTorranceGetSpecular(newMaterial) = mth::vec3f(0.04f, 0.04f, 0.04f);

        // Opacity
        if (assimpMaterial->Get(AI_MATKEY_OPACITY, glossFactor) == aiReturn_SUCCESS)
          GDRGPUMaterialCookTorranceGetOpacity(newMaterial) = glossFactor;
        else
          GDRGPUMaterialCookTorranceGetOpacity(newMaterial) = 1.0f;

        // Specular + Glossiness map
        GDRGPUMaterialCookTorranceGetSpecularGlossinessMapIndex(newMaterial) = GetTextureFromAssimp(assimpMaterial, aiTextureType_SPECULAR);

        // Normal map
        GDRGPUMaterialCookTorranceGetNormalMapIndex(newMaterial) = GetTextureFromAssimp(assimpMaterial, aiTextureType_NORMALS);
    }
    else if (shadingModel == aiShadingMode_PBR_BRDF && (assimpMaterial->Get(AI_MATKEY_METALLIC_FACTOR, metallicFactor) == aiReturn_SUCCESS))
    {
      newMaterial.ShadeType = MATERIAL_SHADER_COOKTORRANCE_METALNESS;

      // Ambient Occlusion
      GDRGPUMaterialCookTorranceGetAmbientOcclusionMapIndex(newMaterial) = GetTextureFromAssimp(assimpMaterial, aiTextureType_LIGHTMAP);

      // Albedo
      GDRGPUMaterialCookTorranceGetAlbedoMapIndex(newMaterial) = GetTextureFromAssimp(assimpMaterial, aiTextureType_BASE_COLOR);
      assimpMaterial->Get(AI_MATKEY_BASE_COLOR, color);
      // HACK
      if (GDRGPUMaterialCookTorranceGetAlbedoMapIndex(newMaterial) != NONE_INDEX && color == aiColor3D(0.f))
          color = aiColor3D(1.f);
      GDRGPUMaterialCookTorranceGetAlbedo(newMaterial) = mth::vec3f(color.r, color.g, color.b);

      // Roughness
      if (assimpMaterial->Get(AI_MATKEY_ROUGHNESS_FACTOR, metallicFactor) == aiReturn_SUCCESS)
          GDRGPUMaterialCookTorranceGetRoughness(newMaterial) = metallicFactor;
      else
          GDRGPUMaterialCookTorranceGetRoughness(newMaterial) = 0.3f;

      // Metallic
      if (assimpMaterial->Get(AI_MATKEY_METALLIC_FACTOR, metallicFactor) == aiReturn_SUCCESS)
          GDRGPUMaterialCookTorranceGetMetalness(newMaterial) = metallicFactor;
      else
          GDRGPUMaterialCookTorranceGetMetalness(newMaterial) = 0.5f;

      // Opacity
      if (assimpMaterial->Get(AI_MATKEY_OPACITY, glossFactor) == aiReturn_SUCCESS)
        GDRGPUMaterialCookTorranceGetOpacity(newMaterial) = glossFactor;
      else
        GDRGPUMaterialCookTorranceGetOpacity(newMaterial) = 1.0f;

      // Metallic + Roughness map
      GDRGPUMaterialCookTorranceGetRoughnessMetalnessMapIndex(newMaterial) = GetTextureFromAssimp(assimpMaterial, aiTextureType_UNKNOWN);

      // Normal map
      GDRGPUMaterialCookTorranceGetNormalMapIndex(newMaterial) = GetTextureFromAssimp(assimpMaterial, aiTextureType_NORMALS);
    }
    else if (shadingModel == aiShadingMode_Phong || shadingModel == aiShadingMode_Blinn)
    {
      newMaterial.ShadeType = MATERIAL_SHADER_PHONG;
      
      // Shiness 
      assimpMaterial->Get(AI_MATKEY_SHININESS, shiness);
      GDRGPUMaterialPhongGetShiness(newMaterial) = shiness / 4.0f;
      assimpMaterial->Get(AI_MATKEY_SHININESS_STRENGTH, shiness);
      GDRGPUMaterialPhongGetShiness(newMaterial) *= shiness;

      // Ka
      {
        GDRGPUMaterialPhongGetAmbientMapIndex(newMaterial) = GetTextureFromAssimp(assimpMaterial, aiTextureType_AMBIENT);
        assimpMaterial->Get(AI_MATKEY_COLOR_AMBIENT, color);
        // HACK
        if (GDRGPUMaterialPhongGetAmbientMapIndex(newMaterial) != NONE_INDEX && color == aiColor3D(0.f))
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
        GDRGPUMaterialPhongGetSpecularMapIndex(newMaterial) = GetTextureFromAssimp(assimpMaterial, aiTextureType_SPECULAR);
        assimpMaterial->Get(AI_MATKEY_COLOR_SPECULAR, color);
        // HACK
        if (GDRGPUMaterialPhongGetSpecularMapIndex(newMaterial) != NONE_INDEX && color == aiColor3D(0.f))
          color = aiColor3D(1.f);
        GDRGPUMaterialPhongGetSpecular(newMaterial) = mth::vec3f(color.r, color.g, color.b);
      }

      // Opacity
      if (assimpMaterial->Get(AI_MATKEY_OPACITY, glossFactor) == aiReturn_SUCCESS)
        GDRGPUMaterialPhongGetOpacity(newMaterial) = glossFactor;
      else
        GDRGPUMaterialPhongGetOpacity(newMaterial) = 1;

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

      // Opacity
      if (assimpMaterial->Get(AI_MATKEY_OPACITY, glossFactor) == aiReturn_SUCCESS)
        GDRGPUMaterialColorGetOpacity(newMaterial) = glossFactor;
      else
        GDRGPUMaterialColorGetOpacity(newMaterial) = 1.0f;
      
      GDRGPUMaterialColorGetColor(newMaterial) = mth::vec3f(color.r, color.g, color.b);
    }
  }

  return Current;
}

bool mesh_assimp_importer::IsTextureTransparent(std::string path)
{
  int w = 0, h = 0, components = 0;
  uint8_t *data = stbi_load(path.c_str(), &w, &h, &components, 0);

  if (components == 1)
  {
    stbi_image_free(data);
    return true;
  }
  else if (components == 2)
  {
    for (int i = 0; i < h; i++)
      for (int j = 0; j < w; j++)
        if (data[i * w * 2 + j * 2 + 1] != 255)
        {
          stbi_image_free(data);
          return true;
        }
    stbi_image_free(data);
    return false;
  }
  else if (components == 3)
  {
    stbi_image_free(data);
    return false;
  }
  else if (components == 4)
  {
    for (int i = 0; i < h; i++)
      for (int j = 0; j < w; j++)
        if (data[i * w * 4 + j * 4 + 3] != 255)
        {
          stbi_image_free(data);
          return true;
        }
    stbi_image_free(data);
    return false;
  }

  GDR_FAILED("Unknown amount of channels");
  stbi_image_free(data);
  return false;
}

void mesh_assimp_importer::CalculateGlobalKeyframes(gdr_index MyIndex)
{
  if (MyIndex == NONE_INDEX)
    return;

  gdr::import_model_node& Me = Result.HierarchyNodes[MyIndex];

  if (Me.Type != gdr_hier_node_type::node)
    return;

  if (Me.ParentIndex == NONE_INDEX)
  {
    Me.GlobalKeyframes = Me.LocalKeyframes;
  }
  else
  {
    gdr::import_model_node& Parent = Result.HierarchyNodes[Me.ParentIndex];
    Me.GlobalKeyframes.resize(Me.LocalKeyframes.size());
    for (int i = 0; i < Parent.GlobalKeyframes.size(); i++)
    {
      mth::matr4f parentGlobalTransform = mth::matr4f::BuildTransform(Parent.GlobalKeyframes[i].scale, Parent.GlobalKeyframes[i].rotationQuat, Parent.GlobalKeyframes[i].pos);
      mth::matr4f localTransform = mth::matr4f::BuildTransform(Me.LocalKeyframes[i].scale, Me.LocalKeyframes[i].rotationQuat, Me.LocalKeyframes[i].pos);
      mth::matr4f globalTransform = localTransform * parentGlobalTransform;
      
      mth::vec3f GlobalPos, GlobalScale;
      mth::vec4f GlobalRot;
      globalTransform.Decompose(GlobalPos, GlobalRot, GlobalScale);

      Me.GlobalKeyframes[i].time = Parent.GlobalKeyframes[i].time;
      Me.GlobalKeyframes[i].pos = GlobalPos;
      Me.GlobalKeyframes[i].rotationQuat = GlobalRot;
      Me.GlobalKeyframes[i].scale = GlobalScale;
    }
  }

  gdr_index NextIndex = Me.ChildIndex;
  while (NextIndex != NONE_INDEX)
  {
    CalculateGlobalKeyframes(NextIndex);
    NextIndex = Result.HierarchyNodes[NextIndex].NextIndex;
  }
}

void mesh_assimp_importer::Import()
{
  scene = importer.ReadFile(FileName,
    aiProcessPreset_TargetRealtime_Fast |
    aiProcess_FlipUVs |
    aiProcess_PopulateArmatureData);
  
  if (!scene)
    return;
  
  Result.FileName = FileName;
  Result.RootTransform.Transform = mth::matr::Identity();
  Result.RootTransform.minAABB = mth::vec3f(FLT_MAX, FLT_MAX, FLT_MAX);
  Result.RootTransform.maxAABB = mth::vec3f(-FLT_MAX, -FLT_MAX, -FLT_MAX);
  
  ImportTreeFirstPass(scene->mRootNode);
  ImportTreeSecondPass(scene->mRootNode);

  /*
  mth::vec3f RotateAxis = {2 * MTH_PI * rand()/ RAND_MAX, 2 * MTH_PI * rand() / RAND_MAX, 2 * MTH_PI * rand() / RAND_MAX };
  mth::vec4f RotateQuaternion = ToQuaternion(RotateAxis);
  mth::matr4f MatrFromAxises = mth::matr4f::RotateZ(RotateAxis.Z) * mth::matr4f::RotateY(RotateAxis.Y) * mth::matr4f::RotateX(RotateAxis.X);
  mth::matr4f MatrFromQuaternion = mth::matr4f::FromQuaternionAndPosition(RotateQuaternion, {0, 0, 0});
  mth::vec3f dummy, dummy2, AxisFromAxis, AxisFromQuat;
  MatrFromAxises.Decompose(dummy, AxisFromAxis, dummy2);
  MatrFromQuaternion.Decompose(dummy, AxisFromQuat, dummy2);
  */
  // Load keyframes
  Result.AnimationDuration = 0;
  if (scene->HasAnimations())
  {
    aiAnimation* anim = scene->mAnimations[0];
    Result.AnimationDuration = anim->mDuration;

    struct import_animation_keyframe
    {
      gdr::animation_keyframe Key;
      bool IsPositionSet = false;
      bool IsRotationSet = false;
      bool IsScaleSet = false;
    };
                     // time                  // node             // keyframe
    std::unordered_map<float, std::unordered_map<int, import_animation_keyframe>> tmpKeyframe;

    // 1) Load all keyframes
    for (int k = 0; k < (int)anim->mNumChannels; k++)
    {
      std::string NodeName = anim->mChannels[k]->mNodeName.C_Str();
      gdr_index our_node_index = NONE_INDEX;
      for (gdr_index i = 0; i < Result.HierarchyNodes.size() && our_node_index == NONE_INDEX; i++)
        if (Result.HierarchyNodes[i].Name == NodeName && Result.HierarchyNodes[i].Type == gdr_hier_node_type::node)
          our_node_index = i;

      if (our_node_index == NONE_INDEX)
      {
        GDR_FAILED("Invalid bone name in animation. Skipping\n");
        continue;
      }
      for (int i = 0; i < anim->mChannels[k]->mNumPositionKeys; i++)
      {
        float time = anim->mChannels[k]->mPositionKeys[i].mTime;
        tmpKeyframe[time][our_node_index].Key.time = time;
        tmpKeyframe[time][our_node_index].Key.pos = {
          anim->mChannels[k]->mPositionKeys[i].mValue[0],
          anim->mChannels[k]->mPositionKeys[i].mValue[1],
          anim->mChannels[k]->mPositionKeys[i].mValue[2] };
        tmpKeyframe[time][our_node_index].IsPositionSet = true;
      }
      for (int i = 0; i < anim->mChannels[k]->mNumRotationKeys; i++)
      {
        float time = anim->mChannels[k]->mRotationKeys[i].mTime;
        tmpKeyframe[time][our_node_index].Key.time = time;
        tmpKeyframe[time][our_node_index].Key.rotationQuat = {
          anim->mChannels[k]->mRotationKeys[i].mValue.x,
          anim->mChannels[k]->mRotationKeys[i].mValue.y,
          anim->mChannels[k]->mRotationKeys[i].mValue.z,
          anim->mChannels[k]->mRotationKeys[i].mValue.w };
        tmpKeyframe[time][our_node_index].IsRotationSet = true;
      }
      for (int i = 0; i < anim->mChannels[k]->mNumScalingKeys; i++)
      {
        float time = anim->mChannels[k]->mScalingKeys[i].mTime;
        tmpKeyframe[time][our_node_index].Key.time = time;
        tmpKeyframe[time][our_node_index].Key.scale = {
           anim->mChannels[k]->mScalingKeys[i].mValue.x,
           anim->mChannels[k]->mScalingKeys[i].mValue.y,
           anim->mChannels[k]->mScalingKeys[i].mValue.z };
        tmpKeyframe[time][our_node_index].IsScaleSet = true;
      }
    }

    // 2) get all times
    std::vector<float> Times;
    for (auto kv : tmpKeyframe)
      Times.push_back(kv.first);

    std::sort(Times.begin(), Times.end());

    // 3) Fill local keyframes
    for (int k = 0; k < (int)anim->mNumChannels; k++)
    {
      std::string NodeName = anim->mChannels[k]->mNodeName.C_Str();
      gdr_index our_node_index = NONE_INDEX;
      for (gdr_index i = 0; i < Result.HierarchyNodes.size() && our_node_index == NONE_INDEX; i++)
        if (Result.HierarchyNodes[i].Name == NodeName && Result.HierarchyNodes[i].Type == gdr_hier_node_type::node)
          our_node_index = i;

      if (our_node_index == NONE_INDEX)
        continue;

      gdr::import_model_node &NodeToAnimate = Result.HierarchyNodes[our_node_index];
      
      mth::vec3f LocalPos, LocalScale;
      mth::vec4f LocalRot;

      NodeToAnimate.LocalTransform.Decompose(LocalPos, LocalRot, LocalScale);

      if (NodeToAnimate.Type != gdr_hier_node_type::node)
        continue;
     
      NodeToAnimate.LocalKeyframes.resize(Times.size());

      for (int i = 0; i < Times.size(); i++)
      {
        import_animation_keyframe &LoadedKeyframe = tmpKeyframe[Times[i]][our_node_index];

        // Fix using interpolation 
        if (!LoadedKeyframe.IsPositionSet)
        {
          GDR_FAILED("NOT IMPLEMENTED");
          int leftIndex = i;
          int rightIndex = i;
          // find left node
          while (leftIndex >= 0 && !tmpKeyframe[Times[leftIndex]][our_node_index].IsPositionSet)
            leftIndex--;

          // find right node
          while (rightIndex < Times.size() && !tmpKeyframe[Times[rightIndex]][our_node_index].IsPositionSet)
            rightIndex++;

          if (leftIndex >= 0 && rightIndex < Times.size())
          {
            float alpha = (Times[i] - Times[leftIndex]) / (Times[rightIndex] - Times[leftIndex]);
            LoadedKeyframe.Key.pos = tmpKeyframe[Times[leftIndex]][our_node_index].Key.pos * (1 - alpha) +
              tmpKeyframe[Times[leftIndex]][our_node_index].Key.pos * alpha;
            LoadedKeyframe.IsPositionSet = true;
          }
          else if (leftIndex >= 0) // if we have left -> copy it
          {
            LoadedKeyframe.Key.pos = tmpKeyframe[Times[leftIndex]][our_node_index].Key.pos;
            LoadedKeyframe.IsPositionSet = true;
          }
          else if (rightIndex < Times.size()) // if we have right -> copy it
          {
            LoadedKeyframe.Key.pos = tmpKeyframe[Times[rightIndex]][our_node_index].Key.pos;
            LoadedKeyframe.IsPositionSet = true;
          }
          else // if nothing -> forget about it
          {
            LoadedKeyframe.Key.pos = LocalPos;
            LoadedKeyframe.IsPositionSet = true;
          }
        }

        // Fix using interpolation 
        if (!LoadedKeyframe.IsRotationSet)
        {
          GDR_FAILED("NOT IMPLEMENTED");
          int leftIndex = i;
          int rightIndex = i;
          // find left node
          while (leftIndex >= 0 && !tmpKeyframe[Times[leftIndex]][our_node_index].IsRotationSet)
            leftIndex--;

          // find right node
          while (rightIndex < Times.size() && !tmpKeyframe[Times[rightIndex]][our_node_index].IsRotationSet)
            rightIndex++;

          if (leftIndex >= 0 && rightIndex < Times.size())
          {
            float alpha = (Times[i] - Times[leftIndex]) / (Times[rightIndex] - Times[leftIndex]);
            LoadedKeyframe.Key.rotationQuat = tmpKeyframe[Times[leftIndex]][our_node_index].Key.rotationQuat.slerp(
              tmpKeyframe[Times[rightIndex]][our_node_index].Key.rotationQuat, alpha);
            LoadedKeyframe.IsRotationSet = true;
          }
          else if (leftIndex >= 0) // if we have left -> copy it
          {
            LoadedKeyframe.Key.rotationQuat = tmpKeyframe[Times[leftIndex]][our_node_index].Key.rotationQuat;
            LoadedKeyframe.IsRotationSet = true;
          }
          else if (rightIndex < Times.size()) // if we have right -> copy it
          {
            LoadedKeyframe.Key.rotationQuat = tmpKeyframe[Times[rightIndex]][our_node_index].Key.rotationQuat;
            LoadedKeyframe.IsRotationSet = true;
          }
          else // if nothing -> forget about it
          {
            LoadedKeyframe.Key.rotationQuat = LocalRot;
            LoadedKeyframe.IsRotationSet = true;
          }
        }

        // Fix using interpolation 
        if (!LoadedKeyframe.IsScaleSet)
        {
          GDR_FAILED("NOT IMPLEMENTED");
          int leftIndex = i;
          int rightIndex = i;
          // find left node
          while (leftIndex >= 0 && !tmpKeyframe[Times[leftIndex]][our_node_index].IsScaleSet)
            leftIndex--;

          // find right node
          while (rightIndex < Times.size() && !tmpKeyframe[Times[rightIndex]][our_node_index].IsScaleSet)
            rightIndex++;

          if (leftIndex >= 0 && rightIndex < Times.size())
          {
            float alpha = (Times[i] - Times[leftIndex]) / (Times[rightIndex] - Times[leftIndex]);
            LoadedKeyframe.Key.scale = tmpKeyframe[Times[leftIndex]][our_node_index].Key.scale * (1-alpha) +
              tmpKeyframe[Times[rightIndex]][our_node_index].Key.rotationQuat * alpha;
            LoadedKeyframe.IsScaleSet = true;
          }
          else if (leftIndex >= 0) // if we have left -> copy it
          {
            LoadedKeyframe.Key.scale = tmpKeyframe[Times[leftIndex]][our_node_index].Key.scale;
            LoadedKeyframe.IsScaleSet = true;
          }
          else if (rightIndex < Times.size()) // if we have right -> copy it
          {
            LoadedKeyframe.Key.scale = tmpKeyframe[Times[rightIndex]][our_node_index].Key.scale;
            LoadedKeyframe.IsScaleSet = true;
          }
          else // if nothing -> forget about it
          {
            LoadedKeyframe.Key.scale = LocalScale;
            LoadedKeyframe.IsScaleSet = true;
          }
        }

        // And copy
        NodeToAnimate.LocalKeyframes[i] = LoadedKeyframe.Key;
      }
    }

    // 4) Fill empty keyframes
    for (int k = 0; k < Result.HierarchyNodes.size(); k++)
    {
      if (Result.HierarchyNodes[k].Type == gdr_hier_node_type::node && Result.HierarchyNodes[k].LocalKeyframes.size() == 0)
      {
        gdr::import_model_node& NodeToAnimate = Result.HierarchyNodes[k];
        mth::vec3f LocalPos, LocalScale;
        mth::vec4f LocalRot;
        NodeToAnimate.LocalTransform.Decompose(LocalPos, LocalRot, LocalScale);
        NodeToAnimate.LocalKeyframes.resize(Times.size());
        for (int l = 0; l < NodeToAnimate.LocalKeyframes.size(); l++)
        {
          NodeToAnimate.LocalKeyframes[l].time = Times[l];
          NodeToAnimate.LocalKeyframes[l].pos = LocalPos;
          NodeToAnimate.LocalKeyframes[l].scale = LocalScale;
          NodeToAnimate.LocalKeyframes[l].rotationQuat = LocalRot;
        }
      }
    }

    // 5) Calculate Global Keyframes
    CalculateGlobalKeyframes(0);

    // 6) Calculate AABB for all frames
    for (int i = 0; i < Result.HierarchyNodes[0].GlobalKeyframes.size(); i++)
      CalculateAABB(0, i);
  }
  else
  {
    // 6) Calculate for root frame
    CalculateAABB(0, NONE_INDEX);
  }

  std::vector<bool> IsTransparent;
  IsTransparent.resize(Result.TexturesPaths.size());
  for (int i = 0; i < Result.TexturesPaths.size(); i++)
    IsTransparent[i] = IsTextureTransparent(Result.TexturesPaths[i]);

  for (int i = 0; i < Result.HierarchyNodes.size(); i++)
    if (Result.HierarchyNodes[i].Type == gdr_hier_node_type::mesh)
    {
      GDRGPUMaterial mat = Result.Materials[Result.HierarchyNodes[i].MaterialIndex];
      gdr_index texture_id = NONE_INDEX;
      float opacity = 1.0;
      if (mat.ShadeType == MATERIAL_SHADER_COLOR)
      {
        texture_id = GDRGPUMaterialColorGetColorMapIndex(mat);
        opacity = GDRGPUMaterialColorGetOpacity(mat);
      }
      else if (mat.ShadeType == MATERIAL_SHADER_PHONG)
      {
        texture_id = GDRGPUMaterialPhongGetDiffuseMapIndex(mat);
        opacity = GDRGPUMaterialPhongGetOpacity(mat);
      }
      else if (mat.ShadeType == MATERIAL_SHADER_COOKTORRANCE_METALNESS || mat.ShadeType == MATERIAL_SHADER_COOKTORRANCE_SPECULAR)
      {
        texture_id = GDRGPUMaterialCookTorranceGetAlbedoMapIndex(mat);
        opacity = GDRGPUMaterialCookTorranceGetOpacity(mat);
      }
      
      if (opacity != 1.0 || (texture_id != NONE_INDEX && IsTransparent[texture_id]))
        Result.HierarchyNodes[i].Params |= OBJECT_PARAMETER_TRANSPARENT;
    }
  importer.FreeScene();
}

void mesh_assimp_importer::ImportSplitted()
{
  // at first -> Load everything
  Import();

  // after that -> Split
  for (int mesh_index = 0; mesh_index < Result.HierarchyNodes.size(); mesh_index++)
    if (Result.HierarchyNodes[mesh_index].Type == gdr_hier_node_type::mesh)
    {
      gdr::import_model_node &MeshNode = Result.HierarchyNodes[mesh_index];
      gdr::model_import_data tmp;
      tmp.FileName = MeshNode.Name + "__"  + Result.FileName;
      
      // Calculate RootTransform
      {
        tmp.RootTransform.Transform = mth::matr::Identity();
        tmp.RootTransform.maxAABB = MeshNode.Vertices[0].Pos;
        tmp.RootTransform.minAABB = MeshNode.Vertices[0].Pos;
        for (int vertex_id = 0; vertex_id < MeshNode.Vertices.size(); vertex_id++)
          for (int component_index = 0; component_index < 3; component_index++)
          {
            tmp.RootTransform.maxAABB[component_index] = max(tmp.RootTransform.maxAABB[component_index], MeshNode.Vertices[vertex_id].Pos[component_index]);
            tmp.RootTransform.minAABB[component_index] = min(tmp.RootTransform.minAABB[component_index], MeshNode.Vertices[vertex_id].Pos[component_index]);
          }

        GDR_ASSERT(Result.HierarchyNodes[mesh_index].BonesMapping.size() <= 1);
        gdr_index node_to_mulipty = Result.HierarchyNodes[mesh_index].BonesMapping[0];
        while (node_to_mulipty != NONE_INDEX)
        {
          tmp.RootTransform.Transform = tmp.RootTransform.Transform * Result.HierarchyNodes[node_to_mulipty].LocalTransform;
          node_to_mulipty = Result.HierarchyNodes[node_to_mulipty].ParentIndex;
        }
      }

      // Copy Mesh
      {
        tmp.HierarchyNodes.push_back(MeshNode);
        tmp.HierarchyNodes[0].BonesMapping.clear();
        tmp.HierarchyNodes[0].ParentIndex = NONE_INDEX;
        tmp.HierarchyNodes[0].ChildIndex = NONE_INDEX;
        tmp.HierarchyNodes[0].NextIndex = NONE_INDEX;
      }

      // Copy Materials
      {
        if (MeshNode.MaterialIndex != NONE_INDEX)
          tmp.Materials.push_back( Result.Materials[MeshNode.MaterialIndex]);
        tmp.HierarchyNodes[0].MaterialIndex = 0;
      }
      
      // Copy Textures
      tmp.TexturesPaths = Result.TexturesPaths;

      Results.push_back(tmp);
    }
}

gdr::model_import_data gdr::ImportModelFromAssimp(std::string filename)
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
      myImporter.ImportDirectory = filename.substr(0, last_slash_idx);
    }
  }

  myImporter.Import();

  return myImporter.Result;
}

std::vector<gdr::model_import_data> gdr::ImportSplittedModelFromAssimp(std::string filename)
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
      myImporter.ImportDirectory = filename.substr(0, last_slash_idx);
    }
  }

  myImporter.ImportSplitted();

  return myImporter.Results;
}