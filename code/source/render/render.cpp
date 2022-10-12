#include "p_header.h"

// Default constructor
gdr::render::render() : IsInited(false), PlayerCamera(mth::vec3f(0, 0, 1), mth::vec3f(0, 0, 0), mth::vec3f(0, 1, 0), 0.1f, 0.1f, 1000, 10, 10)
{
}

// Create Depth-stencil buffer
bool gdr::render::CreateDepthStencil(void)
{
  D3D12_CLEAR_VALUE depthOptimizedClearValue = {};
  depthOptimizedClearValue.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
  depthOptimizedClearValue.DepthStencil.Depth = 1.0f;
  depthOptimizedClearValue.DepthStencil.Stencil = 0;

  bool res = Device.CreateGPUResource(
    CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_D24_UNORM_S8_UINT, Engine->Width, Engine->Height, 1, 0, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL),
    D3D12_RESOURCE_STATE_DEPTH_WRITE,
    &depthOptimizedClearValue,
    DepthBuffer
  );
  DepthBuffer.Resource->SetName(L"DepthStencil buffer");
  if (res)
  {
    D3D12_DEPTH_STENCIL_VIEW_DESC dsvView = {};
    dsvView.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    dsvView.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
    dsvView.Flags = D3D12_DSV_FLAG_NONE;

    Device.GetDXDevice()->CreateDepthStencilView(DepthBuffer.Resource, &dsvView, DSVHeap->GetCPUDescriptorHandleForHeapStart());
  }

  return res;
}

/* Initialize function
 * ARGUMENTS:
 *      - Engine to init
 *          (engine *) Eng
 * RETURNS: true if success, false otherwise
 */
bool gdr::render::Init(engine* Eng)
{
  Engine = Eng;
  bool localIsInited = true;

  gdr::device_create_params params;
  params.hWnd = Eng->hWnd;

  // init device
  localIsInited = Device.Init(params);
  DeviceFrameCounter = device_time_query(&Device);

  // init depth-stencil
  if (localIsInited)
  {
    D3D12_DESCRIPTOR_HEAP_DESC dsvDesc = {};
    dsvDesc.NumDescriptors = 1;
    dsvDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;

    HRESULT hr = S_OK;
    D3D_CHECK(Device.GetDXDevice()->CreateDescriptorHeap(&dsvDesc, __uuidof(ID3D12DescriptorHeap), (void**)&DSVHeap));
    localIsInited = SUCCEEDED(hr);
    localIsInited = localIsInited && CreateDepthStencil();
  }

  //init subsystems
  if (localIsInited)
  {
    GeometrySystem = new gdr::geometry_support(this);
    ObjectSystem = new gdr::object_support(this);
    GlobalsSystem = new gdr::globals_support(this);
    TransformsSystem = new gdr::transforms_support(this);
    IndirectSystem = new gdr::indirect_support(this);
    MaterialsSystem = new gdr::materials_support(this);
    TexturesSystem = new gdr::textures_support(this);
    CubeTexturesSystem= new gdr::cube_textures_support(this);
    LightsSystem = new gdr::light_sources_support(this);
    RenderTargets = new gdr::render_targets_support(this);
  }

  // init passes
  if (localIsInited)
  {
    Passes.push_back(new debug_pass());
    Passes.push_back(new albedo_pass());
    Passes.push_back(new skybox_pass());
    Passes.push_back(new oit_transparent_pass());
    Passes.push_back(new tonemap_pass());
    //Passes.push_back(new hdr_copy_pass());
    Passes.push_back(new imgui_pass());

    for (auto& pass : Passes)
    {
      pass->SetRender(this);
      pass->Initialize();
    }
  }

  IsInited = localIsInited;

  return IsInited;
}

void gdr::render::AddLambdaForIMGUI(std::function<void(void)> func)
{
  for (auto& pass : Passes)
  {
    if (pass->GetName() == "imgui_pass")
    {
      imgui_pass *real_pass = dynamic_cast<imgui_pass*>(pass);

      real_pass->AddLambda(func);
      break;
    }
  }
}

/* Resize frame function
 * ARGUMENTS:
 *      - new frame width
 *          (UINT) w
 *      - new frame height
 *          (UINT) h
 * RETURNS: None
 */
void gdr::render::Resize(UINT w, UINT h)
{
  Device.ResizeSwapchain(w, h);

  PlayerCamera.Resize(w, h);

  RenderTargets->Resize(w, h);

  if (DepthBuffer.Resource != nullptr)
  {
    D3D12_RESOURCE_DESC desc = DepthBuffer.Resource->GetDesc();
    if (desc.Width != w || desc.Height != h)
    {
      Device.ReleaseGPUResource(DepthBuffer);

      CreateDepthStencil();
    }
  }
}

/* Draw frame function */
void gdr::render::DrawFrame(void)
{
  if (!IsInited)
    return;
  PROFILE_CPU_BEGIN("DrawFrame");
  PROFILE_CPU_BEGIN("Update objects transforms");
  ObjectSystem->UpdateAllNodes();
  PROFILE_CPU_END();

  ID3D12GraphicsCommandList* uploadCommandList;
  GetDevice().BeginUploadCommandList(&uploadCommandList);
  auto updateStart = std::chrono::system_clock::now();
  PROFILE_BEGIN(uploadCommandList, "Update indirect SRVs and UAVs");
  // Update all subsytems except globals, it will be updated in each pass
  IndirectSystem->UpdateGPUData(uploadCommandList);
  PROFILE_END(uploadCommandList);
  PROFILE_BEGIN(uploadCommandList, "Update transforms");
  TransformsSystem->UpdateGPUData(uploadCommandList);
  PROFILE_END(uploadCommandList);
  PROFILE_BEGIN(uploadCommandList, "Update materials");
  MaterialsSystem->UpdateGPUData(uploadCommandList);
  PROFILE_END(uploadCommandList);
  PROFILE_BEGIN(uploadCommandList, "Update Textures");
  TexturesSystem->UpdateGPUData(uploadCommandList);
  PROFILE_END(uploadCommandList);
  PROFILE_BEGIN(uploadCommandList, "Update cube Textures");
  CubeTexturesSystem->UpdateGPUData(uploadCommandList);
  PROFILE_END(uploadCommandList);
  PROFILE_BEGIN(uploadCommandList, "Update Lights");
  LightsSystem->UpdateGPUData(uploadCommandList);
  PROFILE_END(uploadCommandList);
  GetDevice().CloseUploadCommandListBeforeRenderCommandList();
  auto updateEnd = std::chrono::system_clock::now();
  UpdateBuffersTime = std::chrono::duration_cast<std::chrono::nanoseconds>(updateEnd - updateStart).count();

  ID3D12GraphicsCommandList* pCommandList = nullptr;
  ID3D12Resource* pBackBuffer = nullptr;
  D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle;
  if (Device.BeginRenderCommandList(&pCommandList, &pBackBuffer, &rtvHandle))
  {
    auto renderStart = std::chrono::system_clock::now();
    PROFILE_BEGIN(pCommandList, "Frame");
    DeviceFrameCounter.Start(pCommandList);
    HRESULT hr = S_OK;
    D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = DSVHeap->GetCPUDescriptorHandleForHeapStart();
    if (Device.TransitResourceState(pCommandList, pBackBuffer, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET))
    {
      FLOAT clearColor[4] = { 0.f, 0.f, 0.f, 1.0f };
      // Scissor rect
      D3D12_RECT Rect;
      Rect.left = 0;
      Rect.top = 0;
      Rect.bottom = Engine->Height;
      Rect.right = Engine->Width;

      pCommandList->ClearRenderTargetView(rtvHandle, clearColor, 1, &Rect);
      pCommandList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 1, &Rect);

      // Save DepthStencil and Display buffers
      RenderTargets->SaveDepthStencilBuffer(&dsvHandle);
      RenderTargets->SaveDisplayBuffer(&rtvHandle, pBackBuffer);

      RenderTargets->Set(pCommandList, render_targets_enum::target_display);

      assert(Passes.size() >= 1);
      for (int i = 0; i < Passes.size(); i++)
      {
        PROFILE_BEGIN(pCommandList, (Passes[i]->GetName() + " compute call").c_str());
        Passes[i]->CallCompute(pCommandList);
        PROFILE_END(pCommandList);
        PROFILE_BEGIN(pCommandList, (Passes[i]->GetName() + " compute sync").c_str());
        Passes[i]->SyncCompute(pCommandList);
        PROFILE_END(pCommandList);
        if (Params.IsIndirect)
        {
          PROFILE_BEGIN(pCommandList, (Passes[i]->GetName() + " Indirect draw").c_str());
          Passes[i]->CallIndirectDraw(pCommandList);
          PROFILE_END(pCommandList);
        }
        else
        {
          PROFILE_BEGIN(pCommandList, (Passes[i]->GetName() + " Direct draw").c_str());
          Passes[i]->CallDirectDraw(pCommandList);
          PROFILE_END(pCommandList);
        }
      }
      DeviceFrameCounter.Stop(pCommandList);
      Device.TransitResourceState(pCommandList, pBackBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
    }

    PROFILE_END(pCommandList);
    Device.CloseSubmitAndPresentRenderCommandList(false);
    auto renderEnd = std::chrono::system_clock::now();
    DrawFrameTime = std::chrono::duration_cast<std::chrono::nanoseconds>(renderEnd - renderStart).count();
  }
  PROFILE_CPU_END();
}

gdr::engine *gdr::render::GetEngine(void)
{
  return Engine;
}

/* Initialize function
 * ARGUMENTS:
 *      - Engine to init
 *          (engine *) Eng
 * RETURNS: true if success, false otherwise
 */
void gdr::render::Term(void)
{
  if (!IsInited)
    return;

  delete RenderTargets;
  delete LightsSystem;
  delete TexturesSystem;
  delete CubeTexturesSystem;
  delete IndirectSystem;
  delete MaterialsSystem; 
  delete TransformsSystem;
  delete ObjectSystem;
  delete GeometrySystem;
  delete GlobalsSystem;
  for (auto& pass : Passes)
  {
    delete pass;
  }
  Device.ReleaseGPUResource(DepthBuffer);
  DSVHeap->Release();
  Passes.clear();
  Device.Term();
}

// Default destructor
gdr::render::~render()
{
  Term();
}