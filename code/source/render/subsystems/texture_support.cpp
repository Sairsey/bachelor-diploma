#include "p_header.h"
#include "stb_image.h"
#include "stb_image_resize.h"


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
gdr::textures_support::textures_support(render* Rnd)
{
  Render = Rnd;
  Render->GetDevice().AllocateStaticDescriptors(MAX_TEXTURE_AMOUNT, TextureTableCPU, TextureTableGPU);
}

// Load Texture
int gdr::textures_support::Load(std::string name, bool isSrgb)
{
  // check if we already have this texture
  for (int i = 0; i < MAX_TEXTURE_AMOUNT; i++)
    if (CPUPool[i].IsUsed && CPUPool[i].Name == name)
      return i;

  int NewTextureIndex = -1;
  for (int i = 0; i < MAX_TEXTURE_AMOUNT; i++)
    if (!CPUPool[i].IsUsed)
    {
      NewTextureIndex = i;
      break;
    }

  if (NewTextureIndex != -1)
  {
    int width, height;
    int components;

    UINT8* stb_buffer = stbi_load(name.c_str(), &width, &height, &components, 0);
    if (stb_buffer != NULL)
    {
      // resize image
      {
        int new_width = NearestPowerOf2(width);
        int new_height = NearestPowerOf2(height);
        UINT8* new_stb_buffer = (UINT8 *)malloc(new_width * new_height * components);
        stbir_resize_uint8(stb_buffer, width, height, 0, new_stb_buffer, new_width, new_height, 0, components);
        stbi_image_free(stb_buffer);

        stb_buffer = new_stb_buffer;
        width = new_width;
        height = new_height;
      }

      UINT mips = 0;
      size_t dataSize = CalculateSizeWithMips(width, height, mips);

      UINT8* pBuffer = new UINT8[dataSize];

      if (components == 4)
        memcpy(pBuffer, stb_buffer, width * height * 4);
      else
        for (int y = 0; y < height; y++)
          for (int x = 0; x < width; x++)
            for (int c = 0; c < 4; c++)
              if (c < components)
                pBuffer[y * width * 4 + x * 4 + c] = stb_buffer[y * width * components + x * components + c];
              else
                pBuffer[y * width * 4 + x * 4 + c] = 255;

      if (components == 4)
      {
        for (int y = 0; y < height && !CPUPool[NewTextureIndex].IsTransparent; y++)
          for (int x = 0; x < width && !CPUPool[NewTextureIndex].IsTransparent; x++)
            // if we have any semi-transparent pixel
            if (pBuffer[y * width * 4 + x * 4 + 3] != 255)
            {
              CPUPool[NewTextureIndex].IsTransparent = true;
            }
      }

      GenerateMips(pBuffer, width, height, mips - 1);
      HRESULT hr = Render->GetDevice().CreateGPUResource(
        CD3DX12_RESOURCE_DESC::Tex2D(
        isSrgb ? DXGI_FORMAT_R8G8B8A8_UNORM_SRGB : DXGI_FORMAT_R8G8B8A8_UNORM,
        width, height, 1, mips),
        D3D12_RESOURCE_STATE_COMMON,
        nullptr,
        CPUPool[NewTextureIndex].TextureResource,
        pBuffer,
        dataSize);
      if (SUCCEEDED(hr))
      {
        CPUPool[NewTextureIndex].Name = name;
        CPUPool[NewTextureIndex].W = width;
        CPUPool[NewTextureIndex].H = height;
        CPUPool[NewTextureIndex].NumOfMips = mips;
        CPUPool[NewTextureIndex].IsUsed = true;
        hr = CPUPool[NewTextureIndex].TextureResource.Resource->SetName(charToWString(name.c_str()).c_str());
      }
      else
      {
        MessageBox(NULL, L"CANNOT allocate texture", L"CANNOT Create geometry", MB_OK);
      }

      stbi_image_free(stb_buffer);
      delete[] pBuffer;
      pBuffer = nullptr;
    }
    else
    {
      printf("Error");
    }
  }
  return NewTextureIndex;
}

// Update data on GPU in case we need it 
void gdr::textures_support::UpdateGPUData(ID3D12GraphicsCommandList* pCommandList)
{
  for (int i = 0; i < MAX_TEXTURE_AMOUNT; i++)
    if (CPUPool[i].IsUsed && !CPUPool[i].IsSrv)
    {
      D3D12_CPU_DESCRIPTOR_HANDLE TextureDescr = TextureTableCPU;

      TextureDescr.ptr += Render->GetDevice().GetSRVDescSize() * i;
      
      D3D12_SHADER_RESOURCE_VIEW_DESC texDesc = {};
      texDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
      texDesc.Format = CPUPool[i].TextureResource.Resource->GetDesc().Format;
      texDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
      texDesc.Texture2D.MipLevels = (UINT)CPUPool[i].NumOfMips;
      Render->GetDevice().GetDXDevice()->CreateShaderResourceView(CPUPool[i].TextureResource.Resource, &texDesc, TextureDescr);
      CPUPool[i].IsSrv = true;
    }
}

// Destructor 
gdr::textures_support::~textures_support(void)
{
  for (int i = 0; i < MAX_TEXTURE_AMOUNT; i++)
    if (CPUPool[i].IsUsed)
      Render->GetDevice().ReleaseGPUResource(CPUPool[i].TextureResource);
}