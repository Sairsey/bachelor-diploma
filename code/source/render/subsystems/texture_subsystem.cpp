#include "p_header.h"
#include "stb_image.h"
#include "stb_image_resize.h"

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
gdr::textures_subsystem::textures_subsystem(render* Rnd) : resource_pool_subsystem(Rnd)
{
  Render->GetDevice().AllocateStaticDescriptors((UINT)Render->CreationParams.MaxTextureAmount, TextureTableCPU, TextureTableGPU);

  SamplersDescs.push_back(CD3DX12_STATIC_SAMPLER_DESC(GDRGPUNearestSamplerSlot, D3D12_FILTER_MIN_MAG_MIP_POINT));
  SamplersDescs.push_back(CD3DX12_STATIC_SAMPLER_DESC(GDRGPULinearSamplerSlot, D3D12_FILTER_MIN_MAG_MIP_LINEAR));
}

// Load Texture
gdr_index gdr::textures_subsystem::Add(std::string name, bool isSrgb)
{
  // check if we already have this texture
  for (int i = 0; i < AllocatedSize(); i++)
    if (IsExist(i) && Get(i).Name == name)
      return i;

  gdr_index NewTextureIndex = resource_pool_subsystem::Add();

  // if too much textures
  if (AllocatedSize() > Render->CreationParams.MaxTextureAmount)
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

    float* stb_bufferf = nullptr;
    UINT8* stb_bufferu = nullptr;
    float* new_stb_bufferf = nullptr;
    UINT8* new_stb_bufferu = nullptr;

    if (isHdr)
      stb_bufferf = stbi_loadf(name.c_str(), &width, &height, &components, 0);
    else
      stb_bufferu = stbi_load(name.c_str(), &width, &height, &components, 0);

    if (stb_bufferf || stb_bufferu)
    {
      // resize image to be power of 2 (needs for mips)
      {
        int new_width = NearestPowerOf2(width);
        int new_height = NearestPowerOf2(height);
        if (isHdr)
        {
          new_stb_bufferf = (float*)malloc(new_width * new_height * components * sizeof(float));
          stbir_resize_float(stb_bufferf, width, height, 0, new_stb_bufferf, new_width, new_height, 0, components);
          stbi_image_free(stb_bufferf);
          stb_bufferf = new_stb_bufferf;
        }
        else
        {
          new_stb_bufferu = (UINT8*)malloc(new_width * new_height * components);
          stbir_resize_uint8(stb_bufferu, width, height, 0, new_stb_bufferu, new_width, new_height, 0, components);
          stbi_image_free(stb_bufferu);
          stb_bufferu = new_stb_bufferu;
        }
        
        width = new_width;
        height = new_height;
      }

      UINT mips = 1;
      size_t dataSize = (isHdr ? width * height * 4 : CalculateSizeWithMips(width, height, mips));

      float* pBufferf = nullptr;
      UINT8* pBufferu = nullptr;

      if (isHdr)
        pBufferf = new float[dataSize];
      else
        pBufferu = new UINT8[dataSize];

      if (components == 4)
      {
        if (isHdr)
          memcpy(pBufferf, stb_bufferf, width * height * 4 * sizeof(float));
        else
          memcpy(pBufferu, stb_bufferu, width * height * 4);
      }
      else if (components == 3) // for monochrome with alpha
      {
        for (int y = 0; y < height; y++)
          for (int x = 0; x < width; x++)
            for (int c = 0; c < 4; c++)
              if (isHdr)
              {
                if (c < components)
                  pBufferf[y * width * 4 + x * 4 + c] = stb_bufferf[y * width * components + x * components + c];
                else
                  pBufferf[y * width * 4 + x * 4 + c] = 1.0;
              }
              else
              {
                if (c < components)
                  pBufferu[y * width * 4 + x * 4 + c] = stb_bufferu[y * width * components + x * components + c];
                else
                  pBufferu[y * width * 4 + x * 4 + c] = 255;
              }
      }
      else if (components == 2) // for monochrome with alpha
      {
        for (int y = 0; y < height; y++)
          for (int x = 0; x < width; x++)
            for (int c = 0; c < 4; c++)
              if (isHdr)
              {
                if (c < 3)
                  pBufferf[y * width * 4 + x * 4 + c] = stb_bufferf[y * width * components + x * components + 0];
                else
                  pBufferf[y * width * 4 + x * 4 + c] = stb_bufferf[y * width * components + x * components + 1];
              }
              else
              {
                if (c < 3)
                  pBufferu[y * width * 4 + x * 4 + c] = stb_bufferu[y * width * components + x * components + 0];
                else
                  pBufferu[y * width * 4 + x * 4 + c] = stb_bufferu[y * width * components + x * components + 1];
              }
      }
      else if (components == 1) // for monochrome
      {
        for (int y = 0; y < height; y++)
          for (int x = 0; x < width; x++)
            for (int c = 0; c < 4; c++)
              if (isHdr)
                pBufferf[y * width * 4 + x * 4 + c] = stb_bufferf[y * width * components + x * components + 0];
              else
                pBufferu[y * width * 4 + x * 4 + c] = stb_bufferu[y * width * components + x * components + 0];
      }

      // if we have any semi-transparent pixel
      if (components == 4 || components == 2)
      {
        for (int y = 0; y < height && !Get(NewTextureIndex).IsTransparent; y++)
          for (int x = 0; x < width && !Get(NewTextureIndex).IsTransparent; x++)
            if ((pBufferf && pBufferf[y * width * 4 + x * 4 + 3] != 1.0) || (pBufferu && pBufferu[y * width * 4 + x * 4 + 3] != 255))
              GetEditable(NewTextureIndex).IsTransparent = true;
      }

      DXGI_FORMAT format = DXGI_FORMAT_R32G32B32A32_FLOAT;
      if (isHdr)
        format = DXGI_FORMAT_R32G32B32A32_FLOAT;
      else if (isSrgb)
        format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
      else 
        format = DXGI_FORMAT_R8G8B8A8_UNORM;

      if (!isHdr)
        GenerateMips(pBufferu, width, height, mips - 1);

      HRESULT hr = Render->GetDevice().CreateGPUResource(
        CD3DX12_RESOURCE_DESC::Tex2D(
          format,
          width, height, 1, mips),
        D3D12_RESOURCE_STATE_COMMON,
        nullptr,
        GetEditable(NewTextureIndex).TextureResource,
        (isHdr ? (void*)pBufferf : (void*)pBufferu),
        dataSize * (isHdr ? sizeof(float) : 1));
      if (SUCCEEDED(hr))
      {
        GetEditable(NewTextureIndex).Name = name;
        GetEditable(NewTextureIndex).W = width;
        GetEditable(NewTextureIndex).H = height;
        GetEditable(NewTextureIndex).NumOfMips = mips;
        GetEditable(NewTextureIndex).IsSrvInited = false;
        GetEditable(NewTextureIndex).TextureResource.Resource->SetName(charToWString(name.c_str()).c_str());
      }
      else
      {
        GDR_FAILED("Failed to allocate DX12 memory for texture");
      }
      if (isHdr)
      {
        stbi_image_free(stb_bufferf);
        delete[] pBufferf;
        pBufferf = nullptr;
      }
      else
      {
        stbi_image_free(stb_bufferu);
        delete[] pBufferu;
        pBufferu = nullptr;
      }
    }
  }
  return NewTextureIndex;
}

// Delete Texture
void gdr::textures_subsystem::Remove(gdr_index index)
{
  if (IsExist(index))
  {
    GetEditable(index).IsSrvInited = false;
    GetEditable(index).Name = "GDR_EMPTY";
    GetEditable(index).W = 0;
    GetEditable(index).H = 0;
    GetEditable(index).NumOfMips = 0;
    GetEditable(index).IsTransparent = 0;
    Render->GetDevice().ReleaseGPUResource(GetEditable(index).TextureResource);
  }
  resource_pool_subsystem::Remove(index);
}

// Update data on GPU in case we need it 
void gdr::textures_subsystem::BeforeUpdateJob(ID3D12GraphicsCommandList* pCommandList)
{
  for (int i = 0; i < AllocatedSize(); i++)
    if (IsExist(i) && !Get(i).IsSrvInited)
    {
      D3D12_CPU_DESCRIPTOR_HANDLE TextureDescr = TextureTableCPU;

      TextureDescr.ptr += Render->GetDevice().GetSRVDescSize() * i;
      
      D3D12_SHADER_RESOURCE_VIEW_DESC texDesc = {};
      texDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
      texDesc.Format = Get(i).TextureResource.Resource->GetDesc().Format;
      texDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
      texDesc.Texture2D.MipLevels = (UINT)Get(i).NumOfMips;
      Render->GetDevice().GetDXDevice()->CreateShaderResourceView(Get(i).TextureResource.Resource, &texDesc, TextureDescr);
      GetEditable(i).IsSrvInited = true;
    }
}

// Destructor 
gdr::textures_subsystem::~textures_subsystem(void)
{
  for (int i = 0; i < AllocatedSize(); i++)
    if (IsExist(i))
      Render->GetDevice().ReleaseGPUResource(GetEditable(i).TextureResource);
}