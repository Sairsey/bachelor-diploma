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

// default constructor
gdr::object_support::object_support(render *Rnd)
{
  Render = Rnd;
}
// function which will import data and return gdr_object
gdr::gdr_object gdr::object_support::CreateObject(const vertex* pVertex, size_t vertexCount, const UINT32* pIndices, size_t indexCount)
{
  Render->GeometrySystem->CreateGeometry(pVertex, vertexCount, pIndices, indexCount);
  
  ObjectTransform newTransform;

  newTransform.maxAABB = pVertex[0].Pos;
  newTransform.minAABB = pVertex[0].Pos;
  for (int i = 0; i < vertexCount; i++)
  {
    newTransform.maxAABB.X = max(newTransform.maxAABB.X, pVertex[i].Pos.X);
    newTransform.maxAABB.Y = max(newTransform.maxAABB.Y, pVertex[i].Pos.Y);
    newTransform.maxAABB.Z = max(newTransform.maxAABB.Z, pVertex[i].Pos.Z);
    newTransform.minAABB.X = min(newTransform.minAABB.X, pVertex[i].Pos.X);
    newTransform.minAABB.Y = min(newTransform.minAABB.Y, pVertex[i].Pos.Y);
    newTransform.minAABB.Z = min(newTransform.minAABB.Z, pVertex[i].Pos.Z);
  }

  newTransform.transform = mth::matr::Identity();
  newTransform.transformInversedTransposed = mth::matr::Identity();
  Render->TransformsSystem->CPUData.push_back(newTransform);

  Render->MaterialsSystem->CPUData.push_back(gdr::materials_support::DefaultMaterial());

  ObjectIndices new_record;
  new_record.ObjectTransformIndex = (UINT)Render->TransformsSystem->CPUData.size() - 1;
  new_record.ObjectMaterialIndex = (UINT)Render->MaterialsSystem->CPUData.size() - 1;
  new_record.ObjectParams = 0;
  CPUPool.push_back(new_record);
  return CPUPool.size() - 1;
}

// function which will import data and return gdr_object
gdr::gdr_object gdr::object_support::DublicateObject(gdr_object original)
{
  Render->GeometrySystem->CPUPool.push_back(Render->GeometrySystem->CPUPool[original]);
  Render->GeometrySystem->CPUPool[Render->GeometrySystem->CPUPool.size() - 1].IsDublicated = true;
  ObjectIndices new_record;
  new_record.ObjectTransformIndex = -1;
  new_record.ObjectMaterialIndex = -1;
  new_record.ObjectParams = 0;

  new_record.ObjectMaterialIndex = CPUPool[original].ObjectMaterialIndex;

  // check transparency
  {
    if (Render->MaterialsSystem->CPUData[new_record.ObjectMaterialIndex].KaMapIndex != -1)
    {
      new_record.ObjectParams |= 
      Render->TexturesSystem->CPUPool[Render->MaterialsSystem->CPUData[new_record.ObjectMaterialIndex].KaMapIndex].IsTransparent 
      ? OBJECT_PARAMETER_TRANSPARENT 
      : 0;
    }
    if (Render->MaterialsSystem->CPUData[new_record.ObjectMaterialIndex].KdMapIndex != -1)
    {
      new_record.ObjectParams |=
        Render->TexturesSystem->CPUPool[Render->MaterialsSystem->CPUData[new_record.ObjectMaterialIndex].KdMapIndex].IsTransparent
        ? OBJECT_PARAMETER_TRANSPARENT
        : 0;
    }
    if (Render->MaterialsSystem->CPUData[new_record.ObjectMaterialIndex].KsMapIndex != -1)
    {
      new_record.ObjectParams |=
        Render->TexturesSystem->CPUPool[Render->MaterialsSystem->CPUData[new_record.ObjectMaterialIndex].KsMapIndex].IsTransparent
        ? OBJECT_PARAMETER_TRANSPARENT
        : 0;
    }
  }

  if (CPUPool[original].ObjectTransformIndex != -1)
  {
    Render->TransformsSystem->CPUData.push_back(Render->TransformsSystem->CPUData[CPUPool[original].ObjectTransformIndex]);
    new_record.ObjectTransformIndex = (UINT)Render->TransformsSystem->CPUData.size() - 1;
  }
  
  CPUPool.push_back(new_record);
  return CPUPool.size() - 1;
}

int gdr::object_support::LoadTextureFromAssimp(aiString *path, aiScene* scene, std::string directory)
{
  int index = -1;
  int textureNumber = path->C_Str()[1] - '0';
  std::string fullpath;
  if (path->C_Str()[0] == '*')
  {
    fullpath = directory + scene->mTextures[textureNumber]->mFilename.C_Str() + "." + scene->mTextures[textureNumber]->achFormatHint;
    FILE *F;
    fopen_s(&F, fullpath.c_str(), "wb");
    fwrite(scene->mTextures[textureNumber]->pcData, 1, scene->mTextures[textureNumber]->mWidth, F);
    fclose(F);
  }
  else
  {
    fullpath = directory + path->C_Str();
  }
  if (path->length != 0)
  {
    index = Render->TexturesSystem->Load(fullpath);
  }

  return index;
}

// function which will import data and return gdr_object
std::vector<gdr::gdr_object> gdr::object_support::CreateObjectsFromFile(std::string fileName)
{
  std::vector<gdr::gdr_object> result;
  // Create an instance of the Importer class
  Assimp::Importer importer;

  if (LoadedObjectTypes.find(fileName) != LoadedObjectTypes.end())
  {
    for (auto &objects : LoadedObjectTypes[fileName])
      result.push_back(DublicateObject(objects));
    return result;
  }

  const aiScene* scene = importer.ReadFile(fileName,
    aiProcessPreset_TargetRealtime_Fast |
    aiProcess_SplitLargeMeshes |
    aiProcess_FlipUVs);

  for (unsigned int meshIndex = 0; meshIndex < scene->mNumMeshes; meshIndex++)
  {
    aiMesh* Mesh = scene->mMeshes[meshIndex];

    std::vector<vertex> vertices;
    std::vector<UINT32> indices;

    vertices.reserve(Mesh->mNumVertices);
    indices.reserve(Mesh->mNumFaces * 3);

    for (int j = 0; j < (int)Mesh->mNumFaces; j++)
    {
      if (Mesh->mFaces[j].mNumIndices != 3)
      {
        continue;
      }
      indices.push_back(Mesh->mFaces[j].mIndices[0]);
      indices.push_back(Mesh->mFaces[j].mIndices[1]);
      indices.push_back(Mesh->mFaces[j].mIndices[2]);
    }

    for (int j = 0; j < (int)Mesh->mNumVertices; j++)
    {
      vertex V;
      V.Pos= mth::vec3f({ Mesh->mVertices[j].x, Mesh->mVertices[j].y, Mesh->mVertices[j].z });
      V.Normal = mth::vec3f({ Mesh->mNormals[j].x, Mesh->mNormals[j].y, Mesh->mNormals[j].z });
      V.Normal.Normalize();
      aiVector3D uv;

      if (Mesh->mTextureCoords[0])
        uv = Mesh->mTextureCoords[0][j];

      V.UV = mth::vec2f({ uv.x, uv.y });
      vertices.push_back(V);
    }

    if (indices.size() == 0)
      continue;

    result.push_back(CreateObject(vertices.data(), vertices.size(), indices.data(), indices.size()));

    ObjectMaterial &mat = GetMaterial(result[result.size() - 1]);

    aiColor3D color(0.f, 0.f, 0.f);
    float shininess = mat.Ph;

    scene->mMaterials[Mesh->mMaterialIndex]->Get(AI_MATKEY_COLOR_DIFFUSE, color);
    mat.Kd = mth::vec3f(color.r, color.g, color.b);

    scene->mMaterials[Mesh->mMaterialIndex]->Get(AI_MATKEY_COLOR_AMBIENT, color);
    mat.Ka = mth::vec3f(color.r, color.g, color.b);

    scene->mMaterials[Mesh->mMaterialIndex]->Get(AI_MATKEY_COLOR_SPECULAR, color);
    mat.Ks = mth::vec3f(color.r, color.g, color.b);

    scene->mMaterials[Mesh->mMaterialIndex]->Get(AI_MATKEY_SHININESS, shininess);
    mat.Ph = shininess / 4.0f;

    if (mat.Ph == 0.f)
      mat.Ph = 1.0f;

    std::string directory;
    size_t last_slash_idx = fileName.rfind('\\');
    if (std::string::npos != last_slash_idx)
    {
      directory = fileName.substr(0, last_slash_idx);
    }
    else
    {
      size_t last_slash_idx = fileName.rfind('/');
      if (std::string::npos != last_slash_idx)
      {
        directory = fileName.substr(0, last_slash_idx) + "/";
      }
    }

    aiShadingMode shadingModel;
    scene->mMaterials[Mesh->mMaterialIndex]->Get(AI_MATKEY_SHADING_MODEL, shadingModel);

    if (shadingModel & aiShadingMode::aiShadingMode_CookTorrance)
    {
      {
        aiString str;
        scene->mMaterials[Mesh->mMaterialIndex]->GetTexture(aiTextureType_DIFFUSE, 0, &str);
        mat.KdMapIndex = LoadTextureFromAssimp(&str, const_cast<aiScene *>(scene), directory);
      }
      {
        aiString str;
        scene->mMaterials[Mesh->mMaterialIndex]->GetTexture(AI_MATKEY_METALLIC_TEXTURE, &str);
        mat.KsMapIndex = LoadTextureFromAssimp(&str, const_cast<aiScene*>(scene), directory);
      }
      mat.ShadeType = MATERIAL_SHADER_COOKTORRANCE;
    }
    else
    {
      {
        aiString str;
        scene->mMaterials[Mesh->mMaterialIndex]->GetTexture(aiTextureType_DIFFUSE, 0, &str);
        mat.KdMapIndex = LoadTextureFromAssimp(&str, const_cast<aiScene*>(scene), directory);
      }
      {
        aiString str;
        scene->mMaterials[Mesh->mMaterialIndex]->GetTexture(aiTextureType_AMBIENT, 0, &str);
        mat.KaMapIndex = LoadTextureFromAssimp(&str, const_cast<aiScene*>(scene), directory);
      }
      {
        aiString str;
        scene->mMaterials[Mesh->mMaterialIndex]->GetTexture(aiTextureType_SPECULAR, 0, &str);
        mat.KsMapIndex = LoadTextureFromAssimp(&str, const_cast<aiScene*>(scene), directory);
      }
      mat.ShadeType = MATERIAL_SHADER_PHONG;
    }

    // check transparency
    if (mat.KdMapIndex != -1)
    {
      CPUPool[CPUPool.size() - 1].ObjectParams |=
        Render->TexturesSystem->CPUPool[mat.KdMapIndex].IsTransparent
        ? OBJECT_PARAMETER_TRANSPARENT
        : 0;
    }
    /*
    if (CPUPool[CPUPool.size() - 1].ObjectParams & OBJECT_PARAMETER_TRANSPARENT)
    {
      mat.ShadeType = MATERIAL_SHADER_DIFFUSE;
    }
    */
  }
  LoadedObjectTypes[fileName] = result;
  
  return result;
}

ObjectTransform& gdr::object_support::GetTransforms(gdr_object object)
{
  return Render->TransformsSystem->CPUData[CPUPool[object].ObjectTransformIndex];
}

// function which will return material by object index
ObjectMaterial& gdr::object_support::GetMaterial(gdr_object object)
{
  return Render->MaterialsSystem->CPUData[CPUPool[object].ObjectMaterialIndex];
}

// destructor
gdr::object_support::~object_support()
{
}