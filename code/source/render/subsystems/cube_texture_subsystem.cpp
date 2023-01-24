#include "p_header.h"
#include "stb_image.h"

// Constructor 
gdr::cube_textures_subsystem::cube_textures_subsystem(render* Rnd) : resource_pool_subsystem(Rnd)
{
  Render->GetDevice().AllocateStaticDescriptors(Render->CreationParams.MaxCubeTextureAmount, CubeTextureTableCPU, CubeTextureTableGPU);
}

// Load Texture
gdr_index gdr::cube_textures_subsystem::Add(
  std::string namePosX,
  std::string nameNegX,
  std::string namePosY,
  std::string nameNegY,
  std::string namePosZ,
  std::string nameNegZ)
{
  std::string name = namePosX + nameNegX + namePosY + nameNegY + namePosZ + nameNegZ;
  // check if we already have this texture
  for (int i = 0; i < AllocatedSize(); i++)
    if (IsExist(i) && Get(i).Name == name)
      return i;

  gdr_index NewTextureIndex = resource_pool_subsystem::Add();

  // if too much textures
  if (AllocatedSize() > Render->CreationParams.MaxCubeTextureAmount)
  {
    resource_pool_subsystem::Remove(NewTextureIndex);
    NewTextureIndex = NONE_INDEX;
  }

  if (NewTextureIndex != NONE_INDEX)
  {
    int width, height;
    int components;

    bool isHdr;

    isHdr = stbi_is_hdr(name.c_str());

    // get width and height
    if (isHdr)
    {
      float* stb_buffer;
      stb_buffer = stbi_loadf(namePosX.c_str(), &width, &height, &components, 0);
      stbi_image_free(stb_buffer);
    }
    else
    {
      UINT8* stb_buffer;
      stb_buffer = stbi_load(namePosX.c_str(), &width, &height, &components, 0);
      stbi_image_free(stb_buffer);
    }

    std::vector<std::string> names;
    names.push_back(namePosX);
    names.push_back(nameNegX);
    names.push_back(namePosY);
    names.push_back(nameNegY);
    names.push_back(namePosZ);
    names.push_back(nameNegZ);

    UINT mips = 1;
    size_t dataSize = width * height * 4 * 6; // width * height * channels * side_amount

    UINT8* pBuffer = nullptr;
    float* pBufferFloat = nullptr;

    if (isHdr)
    {
      pBufferFloat = new float[dataSize];
      float* stb_buffer;
      for (size_t i = 0; i < names.size(); i++)
      {
        stb_buffer = stbi_loadf(names[i].c_str(), &width, &height, &components, 0);
        assert(stb_buffer != NULL);

        if (components == 4)
          memcpy(pBufferFloat + i * height * width * 4, stb_buffer, width * height * 4 * sizeof(float));
        else
          for (int y = 0; y < height; y++)
            for (int x = 0; x < width; x++)
              for (int c = 0; c < 4; c++)
                if (c < components)
                  pBufferFloat[i * height * width * 4 + y * width * 4 + x * 4 + c] = stb_buffer[y * width * components + x * components + c];
                else
                  pBufferFloat[i * height * width * 4 + y * width * 4 + x * 4 + c] = 1.0;

        stbi_image_free(stb_buffer);
      }
    }
    else
    {
      pBuffer = new UINT8[dataSize];
      UINT8* stb_buffer;
      for (size_t i = 0; i < names.size(); i++)
      {
        stb_buffer = stbi_load(names[i].c_str(), &width, &height, &components, 0);
        assert(stb_buffer != NULL);

        if (components == 4)
          memcpy(pBuffer + i * height * width * 4, stb_buffer, width * height * 4 * sizeof(UINT8));
        else
          for (int y = 0; y < height; y++)
            for (int x = 0; x < width; x++)
              for (int c = 0; c < 4; c++)
                if (c < components)
                  pBuffer[i * height * width * 4 + y * width * 4 + x * 4 + c] = stb_buffer[y * width * components + x * components + c];
                else
                  pBuffer[i * height * width * 4 + y * width * 4 + x * 4 + c] = 255;

        stbi_image_free(stb_buffer);
      }
    }

    HRESULT hr = Render->GetDevice().CreateGPUResource(
      CD3DX12_RESOURCE_DESC::Tex2D(
        isHdr ? DXGI_FORMAT_R32G32B32A32_FLOAT : DXGI_FORMAT_R8G8B8A8_UNORM_SRGB,
        width, height, 6, mips),
      D3D12_RESOURCE_STATE_COMMON,
      nullptr,
      GetEditable(NewTextureIndex).TextureResource,
      isHdr ? (UINT8*)pBufferFloat : pBuffer,
      dataSize * (isHdr ? sizeof(float) : sizeof(UINT8)));
    if (SUCCEEDED(hr))
    {
      GetEditable(NewTextureIndex).Name = name;
      GetEditable(NewTextureIndex).IsSrvInited = false;
      GetEditable(NewTextureIndex).TextureResource.Resource->SetName(charToWString(name.c_str()).c_str());
    }
    else
    {
      MessageBox(NULL, L"CANNOT allocate texture", L"CANNOT allocate texture", MB_OK);
    }
    if (isHdr)
      delete[] pBufferFloat;
    else
      delete[] pBuffer;
    pBuffer = nullptr;
    pBufferFloat = nullptr;
  }
  return NewTextureIndex;
}

// Load Texture
gdr_index gdr::cube_textures_subsystem::Add(std::string directory, int MipsAmount)
{
  std::string name = directory;
  // check if we already have this texture
  for (int i = 0; i < AllocatedSize(); i++)
    if (IsExist(i) && Get(i).Name == name)
      return i;

  gdr_index NewTextureIndex = resource_pool_subsystem::Add();

  // if too much textures
  if (AllocatedSize() > Render->CreationParams.MaxCubeTextureAmount)
  {
    resource_pool_subsystem::Remove(NewTextureIndex);
    NewTextureIndex = NONE_INDEX;
  }

  if (NewTextureIndex != NONE_INDEX)
  {
    int maxWidth, maxHeight;
    int components;

    // get width and height
    float* stb_buffer;
    stb_buffer = stbi_loadf((directory + "\\0\\px.hdr").c_str(), &maxWidth, &maxHeight, &components, 0);
    stbi_image_free(stb_buffer);

    // Calculate size
    size_t dataSize = 0;
    int currW = maxWidth, currH = maxHeight;
    for (int i = 0; i < MipsAmount; i++)
    {
      dataSize += currW * currH;
      currW /= 2;
      currH /= 2;
    }
    dataSize *= 4 * sizeof(float) * 6;

    // allocate Buffer
    float* pBufferFloat = nullptr;
    pBufferFloat = new float[dataSize / sizeof(float)];

    // fill names
    std::vector<std::string> names;
    for (int i = 0; i < MipsAmount; i++)
      names.push_back(directory + "\\" + std::to_string(i) + "\\" + "px.hdr");
    for (int i = 0; i < MipsAmount; i++)
      names.push_back(directory + "\\" + std::to_string(i) + "\\" + "nx.hdr");
    for (int i = 0; i < MipsAmount; i++)
      names.push_back(directory + "\\" + std::to_string(i) + "\\" + "py.hdr");
    for (int i = 0; i < MipsAmount; i++)
      names.push_back(directory + "\\" + std::to_string(i) + "\\" + "ny.hdr");
    for (int i = 0; i < MipsAmount; i++)
      names.push_back(directory + "\\" + std::to_string(i) + "\\" + "pz.hdr");
    for (int i = 0; i < MipsAmount; i++)
      names.push_back(directory + "\\" + std::to_string(i) + "\\" + "nz.hdr");

    float* dest_buffer = pBufferFloat;

    for (size_t i = 0; i < names.size(); i++)
    {
      int width, height;
      stb_buffer = stbi_loadf(names[i].c_str(), &width, &height, &components, 0);
      assert(stb_buffer != NULL);

      if (components == 4)
        memcpy(dest_buffer, stb_buffer, width * height * 4 * sizeof(float));
      else
        for (int y = 0; y < height; y++)
          for (int x = 0; x < width; x++)
            for (int c = 0; c < 4; c++)
              if (c < components)
                dest_buffer[y * width * 4 + x * 4 + c] = stb_buffer[y * width * components + x * components + c];
              else
                dest_buffer[y * width * 4 + x * 4 + c] = 1.0;

      stbi_image_free(stb_buffer);
      dest_buffer += width * height * 4;
    }

    HRESULT hr = Render->GetDevice().CreateGPUResource(
      CD3DX12_RESOURCE_DESC::Tex2D(
        DXGI_FORMAT_R32G32B32A32_FLOAT,
        maxWidth, maxHeight, 6, MipsAmount),
      D3D12_RESOURCE_STATE_COMMON,
      nullptr,
      GetEditable(NewTextureIndex).TextureResource,
      pBufferFloat,
      dataSize);
    if (SUCCEEDED(hr))
    {
      GetEditable(NewTextureIndex).Name = name;
      GetEditable(NewTextureIndex).IsSrvInited = false;
      GetEditable(NewTextureIndex).TextureResource.Resource->SetName(charToWString(name.c_str()).c_str());
    }
    else
    {
      MessageBox(NULL, L"CANNOT allocate texture", L"CANNOT allocate texture", MB_OK);
    }

    delete[] pBufferFloat;
    pBufferFloat = nullptr;
  }
  return NewTextureIndex;
}

// Delete Texture
void gdr::cube_textures_subsystem::Remove(gdr_index index)
{
  if (IsExist(index))
  {
    GetEditable(index).IsSrvInited = false;
    GetEditable(index).Name = "GDR_EMPTY";
    Render->GetDevice().ReleaseGPUResource(GetEditable(index).TextureResource);
  }
  resource_pool_subsystem::Remove(index);
}

// Update data on GPU in case we need it 
void gdr::cube_textures_subsystem::BeforeUpdateJob(ID3D12GraphicsCommandList* pCommandList)
{
  for (int i = 0; i < AllocatedSize(); i++)
    if (IsExist(i) && !Get(i).IsSrvInited)
    {
      D3D12_CPU_DESCRIPTOR_HANDLE TextureDescr = CubeTextureTableCPU;

      TextureDescr.ptr += Render->GetDevice().GetSRVDescSize() * i;
      
      D3D12_SHADER_RESOURCE_VIEW_DESC texDesc = {};
      texDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
      texDesc.Format = Get(i).TextureResource.Resource->GetDesc().Format;
      texDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
      texDesc.TextureCube.MipLevels = Get(i).TextureResource.Resource->GetDesc().MipLevels;
      texDesc.TextureCube.MostDetailedMip = 0;
      texDesc.TextureCube.ResourceMinLODClamp = 0.0f;
      Render->GetDevice().GetDXDevice()->CreateShaderResourceView(Get(i).TextureResource.Resource, &texDesc, TextureDescr);
      GetEditable(i).IsSrvInited = true;
    }
}

// Destructor 
gdr::cube_textures_subsystem::~cube_textures_subsystem(void)
{
  for (int i = 0; i < AllocatedSize(); i++)
    if (IsExist(i))
      Render->GetDevice().ReleaseGPUResource(GetEditable(i).TextureResource);
}