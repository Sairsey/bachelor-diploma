#include "p_header.h"
#include "stb_image.h"


// NearestPowerOf2
static UINT NearestPowerOf2(UINT val)
{
  DWORD idx = 0;
  _BitScanReverse(&idx, val);
  UINT res = 1 << idx;

  return (val & ~res) == 0 ? res : (res << 1);
};

static size_t CalculateSizeWithMips(UINT width, UINT height, UINT& mipCount)
{
  DWORD mips = 0;
  _BitScanForward(&mips, min(NearestPowerOf2(height), NearestPowerOf2(width)));

  mipCount = (UINT)mips + 1;
  mipCount -= 2; // Skip last two mips, as texture cannot be less than 4x4 pixels

  size_t stride = width * 4;

  size_t res = 0;
  for (UINT i = 0; i < mipCount; i++)
  {
    res += 1 * height * stride;
    height /= 2;
    stride /= 2;
  }

  return res;
}

static void GenerateMips(void* pInitialData, UINT width, UINT height, UINT mipsToGenerate)
{
  UINT8* pData = static_cast<UINT8*>(pInitialData);

  size_t stride = width * 4;

  for (UINT i = 0; i < mipsToGenerate; i++)
  {
    const UINT8* pSrcData = pData;
    pData += 1 * height * stride;
    height /= 2;
    width /= 2;
    stride /= 2;

    for (UINT y = 0; y < height; y++)
    {
      for (UINT x = 0; x < width; x++)
      {
        for (int i = 0; i < 4; i++)
        {
          int accum = 0;

          accum += *(pSrcData + ((stride * 2) * (y * 2 + 0) + (x * 2 + 0) * 4) * 1 + i);
          accum += *(pSrcData + ((stride * 2) * (y * 2 + 1) + (x * 2 + 0) * 4) * 1 + i);
          accum += *(pSrcData + ((stride * 2) * (y * 2 + 0) + (x * 2 + 1) * 4) * 1 + i);
          accum += *(pSrcData + ((stride * 2) * (y * 2 + 1) + (x * 2 + 1) * 4) * 1 + i);
          accum /= 4;

          pData[(stride * y + x * 4) * 1 + i] = (UINT8)accum;
        }
      }
    }
  }
}

// Constructor
gdr::cube_textures_support::cube_textures_support(render* Rnd)
{
  Render = Rnd;
  Render->GetDevice().AllocateStaticDescriptors(MAX_CUBE_TEXTURE_AMOUNT, CubeTextureTableCPU, CubeTextureTableGPU);
}

// Load Texture
int gdr::cube_textures_support::Load(
  std::string namePosX,
  std::string nameNegX,
  std::string namePosY,
  std::string nameNegY,
  std::string namePosZ,
  std::string nameNegZ)
{
  // check if we already have this texture
  for (int i = 0; i < MAX_CUBE_TEXTURE_AMOUNT; i++)
    if (CPUPool[i].IsUsed && CPUPool[i].Name == namePosX + namePosY + namePosZ + nameNegX + nameNegY + nameNegZ)
      return i;

  int NewTextureIndex = -1;
  for (int i = 0; i < MAX_CUBE_TEXTURE_AMOUNT; i++)
    if (!CPUPool[i].IsUsed)
    {
      NewTextureIndex = i;
      break;
    }

  if (NewTextureIndex != -1)
  {
    int width, height;
    int components;

    bool isHdr;

    isHdr = stbi_is_hdr(namePosX.c_str());

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
    size_t dataSize = width * height * 6 * 4;

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
     CPUPool[NewTextureIndex].TextureResource,
     isHdr ? (UINT8*)pBufferFloat : pBuffer,
     dataSize * (isHdr ? sizeof(float) : sizeof(UINT8)));
    if (SUCCEEDED(hr))
    {
      CPUPool[NewTextureIndex].Name = namePosX + namePosY + namePosZ + nameNegX + nameNegY + nameNegZ;
      CPUPool[NewTextureIndex].IsUsed = true;
      hr = CPUPool[NewTextureIndex].TextureResource.Resource->SetName(charToWString(CPUPool[NewTextureIndex].Name.c_str()).c_str());
    }
    else
    {
      MessageBox(NULL, L"CANNOT allocate texture", L"CANNOT Create geometry", MB_OK);
    }

    if (isHdr)
      delete[] pBufferFloat;
    else
      delete[] pBuffer;
    pBuffer = nullptr;
    pBufferFloat = nullptr;
  }
  else
  {
    printf("Error");
  }
  return NewTextureIndex;
}

// Update data on GPU in case we need it 
void gdr::cube_textures_support::UpdateGPUData(ID3D12GraphicsCommandList* pCommandList)
{
  for (int i = 0; i < MAX_CUBE_TEXTURE_AMOUNT; i++)
    if (CPUPool[i].IsUsed && !CPUPool[i].IsSrv)
    {
      D3D12_CPU_DESCRIPTOR_HANDLE TextureDescr = CubeTextureTableCPU;

      TextureDescr.ptr += Render->GetDevice().GetSRVDescSize() * i;
      
      D3D12_SHADER_RESOURCE_VIEW_DESC texDesc = {};
      texDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
      texDesc.Format = CPUPool[i].TextureResource.Resource->GetDesc().Format;
      texDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
      texDesc.TextureCube.MipLevels = 1;
      texDesc.TextureCube.MostDetailedMip = 0;
      texDesc.TextureCube.ResourceMinLODClamp = 0.0f;
      Render->GetDevice().GetDXDevice()->CreateShaderResourceView(CPUPool[i].TextureResource.Resource, &texDesc, TextureDescr);
      CPUPool[i].IsSrv = true;
    }
}

// Destructor 
gdr::cube_textures_support::~cube_textures_support(void)
{
  for (int i = 0; i < MAX_CUBE_TEXTURE_AMOUNT; i++)
    if (CPUPool[i].IsUsed)
      Render->GetDevice().ReleaseGPUResource(CPUPool[i].TextureResource);
}