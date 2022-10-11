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

ObjectTransform gdr::gdr_node::GetTransform()
{
  return Render->TransformsSystem->CPUData[TransformIndex];
}

mth::matr4f& gdr::gdr_node::GetTransformEditable()
{
  IsTransformCalculated = false;
  if (NodeType == gdr_node_type::mesh)
    return Render->ObjectSystem->NodesPool[ParentIndex].LocalTransform;
  else
    return LocalTransform;
}

ObjectMaterial& gdr::gdr_node::GetMaterial()
{
  assert(NodeType == gdr_node_type::mesh);
  return Render->MaterialsSystem->CPUData[Render->ObjectSystem->CPUPool[MeshIndex].ObjectMaterialIndex];
}

// default constructor
gdr::object_support::object_support(render *Rnd)
{
  Render = Rnd;
}
// function which will import data and return gdr_object
gdr::gdr_index gdr::object_support::CreateObject(const vertex* pVertex, size_t vertexCount, const UINT32* pIndices, size_t indexCount)
{
  gdr_index newMeshIndex = Render->GeometrySystem->CPUPool.size();
  // Load mesh
  Render->GeometrySystem->CreateGeometry(pVertex, vertexCount, pIndices, indexCount);

  // Construct transform
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
  // Construct Material
  Render->MaterialsSystem->CPUData.push_back(gdr::materials_support::DefaultMaterial());
  // Construct Indices
  ObjectIndices new_record;
  new_record.ObjectTransformIndex = (UINT)Render->TransformsSystem->CPUData.size() - 1;
  new_record.ObjectMaterialIndex = (UINT)Render->MaterialsSystem->CPUData.size() - 1;
  new_record.ObjectParams = 0;
  CPUPool.push_back(new_record);

  // Construct gdr_index
  gdr_node Object(Render);
  Object.Name = "Constructed object No.";
  Object.Name += std::to_string(NodesPool.size());
  Object.ParentIndex = -1;
  Object.Index = NodesPool.size();

  Object.TransformIndex = Render->TransformsSystem->CPUData.size() - 1;
  Object.NodeType = gdr_node_type::mesh;

  // Transforms for hierarchic computation
  Object.LocalTransform = mth::matr4f::Identity();
  Object.GlobalTransform = mth::matr4f::Identity();
  Object.IsTransformCalculated = true;

  Object.MeshIndex = CPUPool.size() - 1;
  NodesPool.push_back(Object);

  return NodesPool.size() - 1;
}

// function which will import data and return gdr_object
/*gdr::gdr_object gdr::object_support::DublicateObject(gdr_object original)
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
}*/

int gdr::object_support::LoadTextureFromAssimp(aiString *path, aiScene* scene, std::string directory, bool isSrgb)
{
  int index = -1;
  int textureNumber = atoi(path->C_Str() + 1);
  std::string fullpath;
  if (path->C_Str()[0] == '*')
  {
    fullpath = directory + scene->mTextures[textureNumber]->mFilename.C_Str() + (path->C_Str() + 1) + "." + scene->mTextures[textureNumber]->achFormatHint;
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
    index = Render->TexturesSystem->Load(fullpath, isSrgb);
  }

  return index;
}

// Load assimp tree recursive function
gdr::gdr_index gdr::object_support::LoadAssimpTree(const aiScene* scene, aiNode* node, gdr_index ParentNode)
{
  // Construct transform
  ObjectTransform newTransform;
  newTransform.maxAABB = 0;
  newTransform.minAABB = 0;
  newTransform.transform = mth::matr::Identity();
  newTransform.transformInversedTransposed = mth::matr::Identity();
  Render->TransformsSystem->CPUData.push_back(newTransform);

  // Load this node in Pool
  gdr_node Node(Render);
  Node.Name = node->mName.C_Str();
  Node.Index = NodesPool.size();
  Node.ParentIndex = ParentNode;
  Node.NodeType = gdr_node_type::node;
  Node.TransformIndex = Render->TransformsSystem->CPUData.size() - 1.0;
  Node.LocalTransform = mth::matr4f(
    node->mTransformation.a1, node->mTransformation.b1, node->mTransformation.c1, node->mTransformation.d1,
    node->mTransformation.a2, node->mTransformation.b2, node->mTransformation.c2, node->mTransformation.d2,
    node->mTransformation.a3, node->mTransformation.b3, node->mTransformation.c3, node->mTransformation.d3,
    node->mTransformation.a4, node->mTransformation.b4, node->mTransformation.c4, node->mTransformation.d4);
  Node.GlobalTransform = mth::matr4f::Identity();
  NodesPool.push_back(Node);

  // for all meshes
  for (unsigned int i = 0; i < node->mNumMeshes; i++)
  {
    gdr_index res = LoadAssimpTreeMesh(scene, scene->mMeshes[node->mMeshes[i]], Node.Index);
    if (res != -1)
      NodesPool[Node.Index].Childs.push_back(res);
  }

  // for all nodes
  for (unsigned int i = 0; i < node->mNumChildren; i++)
  {
    NodesPool[Node.Index].Childs.push_back(LoadAssimpTree(scene, node->mChildren[i], Node.Index));
    Render->TransformsSystem->CPUData[NodesPool[Node.Index].TransformIndex].maxAABB.X = max(NodesPool[NodesPool[Node.Index].Childs[i]].GetTransform().maxAABB.X, NodesPool[Node.Index].GetTransform().maxAABB.X);
    Render->TransformsSystem->CPUData[NodesPool[Node.Index].TransformIndex].maxAABB.Y = max(NodesPool[NodesPool[Node.Index].Childs[i]].GetTransform().maxAABB.Y, NodesPool[Node.Index].GetTransform().maxAABB.Y);
    Render->TransformsSystem->CPUData[NodesPool[Node.Index].TransformIndex].maxAABB.Z = max(NodesPool[NodesPool[Node.Index].Childs[i]].GetTransform().maxAABB.Z, NodesPool[Node.Index].GetTransform().maxAABB.Z);
    Render->TransformsSystem->CPUData[NodesPool[Node.Index].TransformIndex].minAABB.X = min(NodesPool[NodesPool[Node.Index].Childs[i]].GetTransform().minAABB.X, NodesPool[Node.Index].GetTransform().minAABB.X);
    Render->TransformsSystem->CPUData[NodesPool[Node.Index].TransformIndex].minAABB.Y = min(NodesPool[NodesPool[Node.Index].Childs[i]].GetTransform().minAABB.Y, NodesPool[Node.Index].GetTransform().minAABB.Y);
    Render->TransformsSystem->CPUData[NodesPool[Node.Index].TransformIndex].minAABB.Z = min(NodesPool[NodesPool[Node.Index].Childs[i]].GetTransform().minAABB.Z, NodesPool[Node.Index].GetTransform().minAABB.Z);
  }

  return Node.Index;
}

// Load assimp tree recursive function
gdr::gdr_index gdr::object_support::LoadAssimpTreeMesh(const aiScene* scene, aiMesh* mesh, gdr_index ParentNode)
{
  std::vector<vertex> vertices;
  std::vector<UINT32> indices;

  vertices.reserve(mesh->mNumVertices);
  indices.reserve(mesh->mNumFaces * 3);

  for (int j = 0; j < (int)mesh->mNumFaces; j++)
  {
    if (mesh->mFaces[j].mNumIndices != 3)
    {
      continue;
    }
    indices.push_back(mesh->mFaces[j].mIndices[0]);
    indices.push_back(mesh->mFaces[j].mIndices[1]);
    indices.push_back(mesh->mFaces[j].mIndices[2]);
  }

  if (indices.size() == 0)
    return -1;

  for (int j = 0; j < (int)mesh->mNumVertices; j++)
  {
    vertex V;
    V.Pos = mth::vec3f({ mesh->mVertices[j].x, mesh->mVertices[j].y, mesh->mVertices[j].z });
    V.Normal = mth::vec3f({ mesh->mNormals[j].x, mesh->mNormals[j].y, mesh->mNormals[j].z });
    V.Normal.Normalize();
    aiVector3D uv;

    if (mesh->mTextureCoords[0])
      uv = mesh->mTextureCoords[0][j];

    V.UV = mth::vec2f({ uv.x, uv.y });

    V.Tangent = mth::vec3f{ mesh->mTangents[j].x, mesh->mTangents[j].y, mesh->mTangents[j].z };
    V.Tangent.Normalize();

    vertices.push_back(V);
  }

  // Create object with some nodes
  gdr_index object_node = CreateObject(vertices.data(), vertices.size(), indices.data(), indices.size());

  // First - update info about AABB into parent Node
  Render->TransformsSystem->CPUData[NodesPool[ParentNode].TransformIndex].maxAABB.X = max(NodesPool[object_node].GetTransform().maxAABB.X, NodesPool[ParentNode].GetTransform().maxAABB.X);
  Render->TransformsSystem->CPUData[NodesPool[ParentNode].TransformIndex].maxAABB.Y = max(NodesPool[object_node].GetTransform().maxAABB.Y, NodesPool[ParentNode].GetTransform().maxAABB.Y);
  Render->TransformsSystem->CPUData[NodesPool[ParentNode].TransformIndex].maxAABB.Z = max(NodesPool[object_node].GetTransform().maxAABB.Z, NodesPool[ParentNode].GetTransform().maxAABB.Z);
  Render->TransformsSystem->CPUData[NodesPool[ParentNode].TransformIndex].minAABB.X = min(NodesPool[object_node].GetTransform().minAABB.X, NodesPool[ParentNode].GetTransform().minAABB.X);
  Render->TransformsSystem->CPUData[NodesPool[ParentNode].TransformIndex].minAABB.Y = min(NodesPool[object_node].GetTransform().minAABB.Y, NodesPool[ParentNode].GetTransform().minAABB.Y);
  Render->TransformsSystem->CPUData[NodesPool[ParentNode].TransformIndex].minAABB.Z = min(NodesPool[object_node].GetTransform().minAABB.Z, NodesPool[ParentNode].GetTransform().minAABB.Z);
  
  // Second - delete Transform from TransformsPool
  Render->TransformsSystem->CPUData.pop_back();

  // Third - fill new data into object
  NodesPool[object_node].Name = mesh->mName.C_Str();
  NodesPool[object_node].ParentIndex = ParentNode;
  NodesPool[object_node].TransformIndex = NodesPool[ParentNode].TransformIndex;
  NodesPool[object_node].LocalTransform = mth::matr4f::Identity();
  NodesPool[object_node].GlobalTransform = mth::matr4f::Identity();
  NodesPool[object_node].IsTransformCalculated = false;
  CPUPool[NodesPool[object_node].MeshIndex].ObjectTransformIndex = NodesPool[ParentNode].TransformIndex;
  
  ObjectMaterial& mat = NodesPool[object_node].GetMaterial();

  aiColor3D color(0.f, 0.f, 0.f);
  float shininess = mat.Ph;

  scene->mMaterials[mesh->mMaterialIndex]->Get(AI_MATKEY_COLOR_DIFFUSE, color);
  mat.Kd = mth::vec3f(color.r, color.g, color.b);

  scene->mMaterials[mesh->mMaterialIndex]->Get(AI_MATKEY_COLOR_AMBIENT, color);
  mat.Ka = mth::vec3f(color.r, color.g, color.b);

  scene->mMaterials[mesh->mMaterialIndex]->Get(AI_MATKEY_COLOR_SPECULAR, color);
  mat.Ks = mth::vec3f(color.r, color.g, color.b);

  scene->mMaterials[mesh->mMaterialIndex]->Get(AI_MATKEY_SHININESS, shininess);
  mat.Ph = shininess / 4.0f;

  if (mat.Ph == 0.f)
    mat.Ph = 1.0f;

  aiShadingMode shadingModel;
  scene->mMaterials[mesh->mMaterialIndex]->Get(AI_MATKEY_SHADING_MODEL, shadingModel);

  if (shadingModel & aiShadingMode::aiShadingMode_CookTorrance)
  {
    {
      aiString str;
      scene->mMaterials[mesh->mMaterialIndex]->GetTexture(aiTextureType_DIFFUSE, 0, &str);
      mat.KdMapIndex = LoadTextureFromAssimp(&str, const_cast<aiScene*>(scene), directory, true);
    }
    {
      aiString str;
      scene->mMaterials[mesh->mMaterialIndex]->GetTexture(AI_MATKEY_METALLIC_TEXTURE, &str);
      mat.KsMapIndex = LoadTextureFromAssimp(&str, const_cast<aiScene*>(scene), directory);
    }


    scene->mMaterials[mesh->mMaterialIndex]->Get(AI_MATKEY_METALLIC_FACTOR, mat.Ks.B);
    scene->mMaterials[mesh->mMaterialIndex]->Get(AI_MATKEY_ROUGHNESS_FACTOR, mat.Ks.G);

    mat.ShadeType = MATERIAL_SHADER_COOKTORRANCE;
  }
  else
  {
    {
      aiString str;
      scene->mMaterials[mesh->mMaterialIndex]->GetTexture(aiTextureType_DIFFUSE, 0, &str);
      mat.KdMapIndex = LoadTextureFromAssimp(&str, const_cast<aiScene*>(scene), directory, true);
    }
    {
      aiString str;
      scene->mMaterials[mesh->mMaterialIndex]->GetTexture(aiTextureType_AMBIENT, 0, &str);
      mat.KaMapIndex = LoadTextureFromAssimp(&str, const_cast<aiScene*>(scene), directory);
    }
    {
      aiString str;
      scene->mMaterials[mesh->mMaterialIndex]->GetTexture(aiTextureType_SPECULAR, 0, &str);
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

  // check normalmap
  {
    aiString str;
    scene->mMaterials[mesh->mMaterialIndex]->GetTexture(aiTextureType_NORMALS, 0, &str);
    mat.NormalMapIndex = LoadTextureFromAssimp(&str, const_cast<aiScene*>(scene), directory);
  }

  return object_node;
}

// function which will import data and return gdr_object
gdr::gdr_index gdr::object_support::CreateObjectFromFile(std::string fileName)
{
  // Create an instance of the Importer class
  Assimp::Importer importer;

  /*
  if (LoadedObjectTypes.find(fileName) != LoadedObjectTypes.end())
  {
    for (auto &objects : LoadedObjectTypes[fileName])
      result.push_back(DublicateObject(objects));
    return result;
  }
  */

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

  const aiScene* scene = importer.ReadFile(fileName,
    aiProcessPreset_TargetRealtime_Fast |
    aiProcess_SplitLargeMeshes |
    aiProcess_FlipUVs);

  // Construct default transform
  ObjectTransform newTransform;
  newTransform.maxAABB = 0;
  newTransform.minAABB = 0;
  newTransform.transform = mth::matr::Identity();
  newTransform.transformInversedTransposed = mth::matr::Identity();
  Render->TransformsSystem->CPUData.push_back(newTransform);


  // create node for this scene
  gdr_node FileNode(Render);
  FileNode.Name = fileName;
  FileNode.ParentIndex = -1;
  FileNode.Index = NodesPool.size();
  FileNode.NodeType = gdr_node_type::node;
  FileNode.TransformIndex = Render->TransformsSystem->CPUData.size() - 1.0;
  FileNode.LocalTransform = mth::matr4f::Identity();
  FileNode.GlobalTransform = mth::matr4f::Identity();
  FileNode.IsTransformCalculated = false;
  NodesPool.push_back(FileNode);

  NodesPool[FileNode.Index].Childs.push_back(LoadAssimpTree(scene, scene->mRootNode, FileNode.Index));

  Render->TransformsSystem->CPUData[NodesPool[FileNode.Index].TransformIndex].maxAABB.X = max(NodesPool[NodesPool[FileNode.Index].Childs[0]].GetTransform().maxAABB.X, NodesPool[FileNode.Index].GetTransform().maxAABB.X);
  Render->TransformsSystem->CPUData[NodesPool[FileNode.Index].TransformIndex].maxAABB.Y = max(NodesPool[NodesPool[FileNode.Index].Childs[0]].GetTransform().maxAABB.Y, NodesPool[FileNode.Index].GetTransform().maxAABB.Y);
  Render->TransformsSystem->CPUData[NodesPool[FileNode.Index].TransformIndex].maxAABB.Z = max(NodesPool[NodesPool[FileNode.Index].Childs[0]].GetTransform().maxAABB.Z, NodesPool[FileNode.Index].GetTransform().maxAABB.Z);
  Render->TransformsSystem->CPUData[NodesPool[FileNode.Index].TransformIndex].minAABB.X = min(NodesPool[NodesPool[FileNode.Index].Childs[0]].GetTransform().minAABB.X, NodesPool[FileNode.Index].GetTransform().minAABB.X);
  Render->TransformsSystem->CPUData[NodesPool[FileNode.Index].TransformIndex].minAABB.Y = min(NodesPool[NodesPool[FileNode.Index].Childs[0]].GetTransform().minAABB.Y, NodesPool[FileNode.Index].GetTransform().minAABB.Y);
  Render->TransformsSystem->CPUData[NodesPool[FileNode.Index].TransformIndex].minAABB.Z = min(NodesPool[NodesPool[FileNode.Index].Childs[0]].GetTransform().minAABB.Z, NodesPool[FileNode.Index].GetTransform().minAABB.Z);

  return FileNode.Index;

  /*

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

      V.Tangent = mth::vec3f{Mesh->mTangents[j].x, Mesh->mTangents[j].y, Mesh->mTangents[j].z};
      V.Tangent.Normalize();

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
        mat.KdMapIndex = LoadTextureFromAssimp(&str, const_cast<aiScene *>(scene), directory, true);
      }
      {
        aiString str;
        scene->mMaterials[Mesh->mMaterialIndex]->GetTexture(AI_MATKEY_METALLIC_TEXTURE, &str);
        mat.KsMapIndex = LoadTextureFromAssimp(&str, const_cast<aiScene*>(scene), directory);
      }


      scene->mMaterials[Mesh->mMaterialIndex]->Get(AI_MATKEY_METALLIC_FACTOR, mat.Ks.B);
      scene->mMaterials[Mesh->mMaterialIndex]->Get(AI_MATKEY_ROUGHNESS_FACTOR, mat.Ks.G);
      mat.Ks.B = max(0.001, mat.Ks.B);

      mat.ShadeType = MATERIAL_SHADER_COOKTORRANCE;
    }
    else
    {
      {
        aiString str;
        scene->mMaterials[Mesh->mMaterialIndex]->GetTexture(aiTextureType_DIFFUSE, 0, &str);
        mat.KdMapIndex = LoadTextureFromAssimp(&str, const_cast<aiScene*>(scene), directory, true);
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

    // check normalmap
    {
      aiString str;
      scene->mMaterials[Mesh->mMaterialIndex]->GetTexture(aiTextureType_NORMALS, 0, &str);
      mat.NormalMapIndex = LoadTextureFromAssimp(&str, const_cast<aiScene*>(scene), directory);
    }
  }
  LoadedObjectTypes[fileName] = result;
  
  return result;
  */
}

// Recompute Translations and AABBs
void gdr::object_support::UpdateNode(gdr_index nodeIndex)
{
  if (NodesPool[nodeIndex].ParentIndex == -1)
    NodesPool[nodeIndex].GlobalTransform = NodesPool[nodeIndex].LocalTransform;
  else
    NodesPool[nodeIndex].GlobalTransform = NodesPool[nodeIndex].LocalTransform * NodesPool[NodesPool[nodeIndex].ParentIndex].GlobalTransform;
  Render->TransformsSystem->CPUData[NodesPool[nodeIndex].TransformIndex].transform = NodesPool[nodeIndex].GlobalTransform;
  for (gdr_index i = 0; i < NodesPool[nodeIndex].Childs.size(); i++)
    UpdateNode(NodesPool[nodeIndex].Childs[i]);
  NodesPool[nodeIndex].IsTransformCalculated = true;
}

// Recompute Translations and AABBs
void gdr::object_support::UpdateAllNodes(void)
{
  for (gdr_index startNodeIndex = 0; startNodeIndex < NodesPool.size(); startNodeIndex++)
    if (NodesPool[startNodeIndex].IsTransformCalculated == false)
      UpdateNode(startNodeIndex);
}

/*
ObjectTransform& gdr::object_support::GetTransforms(gdr_object object)
{
  return Render->TransformsSystem->CPUData[CPUPool[object].ObjectTransformIndex];
}

// function which will return material by object index
ObjectMaterial& gdr::object_support::GetMaterial(gdr_object object)
{
  return Render->MaterialsSystem->CPUData[CPUPool[object].ObjectMaterialIndex];
}
*/

// destructor
gdr::object_support::~object_support()
{
}