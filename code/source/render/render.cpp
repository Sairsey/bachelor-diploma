#include "p_header.h"

// Default constructor
gdr::render::render() : IsInited(false), PlayerCamera(mth::vec3f(0, 0, 1), mth::vec3f(0, 0, 0), mth::vec3f(0, 1, 0), 0.1f, 0.1f, 1000, 10, 10)
{
}

// Create Depth-stencil buffer
bool gdr::render::CreateDepthStencil(void)
{
  D3D12_CLEAR_VALUE depthOptimizedClearValue = {};
  depthOptimizedClearValue.Format = DXGI_FORMAT_D32_FLOAT;
  depthOptimizedClearValue.DepthStencil.Depth = 1.0f;
  depthOptimizedClearValue.DepthStencil.Stencil = 0;

  bool res = Device.CreateGPUResource(
    CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_D32_FLOAT, Engine->Width, Engine->Height, 1, 0, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL),
    D3D12_RESOURCE_STATE_DEPTH_WRITE,
    &depthOptimizedClearValue,
    DepthBuffer
  );
  DepthBuffer.Resource->SetName(L"DepthStencil buffer");
  if (res)
  {
    D3D12_DEPTH_STENCIL_VIEW_DESC dsvView = {};
    dsvView.Format = DXGI_FORMAT_D32_FLOAT;
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

  // init subsystems
  if (localIsInited) 
  {
      GlobalsSystem = new globals_subsystem(this);
      RenderTargetsSystem = new render_targets_subsystem(this);
      ObjectTransformsSystem = new object_transforms_subsystem(this);
      NodeTransformsSystem = new node_transforms_subsystem(this);
      DrawCommandsSystem = new draw_commands_subsystem(this);
      GeometrySystem = new geometry_subsystem(this);
      MaterialsSystem = new materials_subsystem(this);
  }

  // init passes
  if (localIsInited)
  {
    Passes.push_back(new albedo_pass());
    Passes.push_back(new imgui_pass());
    for (auto& pass : Passes)
    {
      pass->SetRender(this);
      pass->DeviceTimeCounter = device_time_query(&Device);
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

  RenderTargetsSystem->Resize(w, h);

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
  PROFILE_CPU_BEGIN("gdr::render::DrawFrame");

  ID3D12GraphicsCommandList* uploadCommandList;
  GetDevice().BeginUploadCommandList(&uploadCommandList);
  {
    PROFILE_BEGIN(uploadCommandList, "Update Globals");
    GlobalsSystem->CPUData.VP = PlayerCamera.GetVP(); // camera view-proj
    GlobalsSystem->CPUData.CameraPos = PlayerCamera.GetPos(); // Camera position
    GlobalsSystem->CPUData.time = Engine->GetGlobalTime(); // Time in seconds

    GlobalsSystem->CPUData.DeltaTime = Engine->GetDeltaTime(); // Delta time in seconds	
    GlobalsSystem->CPUData.width = Engine->Width;  // Screen size 
    GlobalsSystem->CPUData.height = Engine->Height; // Screen size 
    GlobalsSystem->UpdateGPUData(uploadCommandList);
    PROFILE_END(uploadCommandList);
  }
  {
    PROFILE_BEGIN(uploadCommandList, "Update Node Transforms");
    NodeTransformsSystem->UpdateGPUData(uploadCommandList);
    PROFILE_END(uploadCommandList);
  }
  {
    PROFILE_BEGIN(uploadCommandList, "Update Object Transforms");
    ObjectTransformsSystem->UpdateGPUData(uploadCommandList);
    PROFILE_END(uploadCommandList);
  }
  {
    PROFILE_BEGIN(uploadCommandList, "Update Materials");
    MaterialsSystem->UpdateGPUData(uploadCommandList);
    PROFILE_END(uploadCommandList);
  }
  GetDevice().CloseUploadCommandListBeforeRenderCommandList();
  
  ID3D12GraphicsCommandList* pCommandList = nullptr;
  ID3D12Resource* pBackBuffer = nullptr;
  D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle;
  if (Device.BeginRenderCommandList(&pCommandList, &pBackBuffer, &rtvHandle))
  {
    {
        GetDevice().SetCommandListAsUpload(pCommandList);
        PROFILE_BEGIN(pCommandList, "Update Indirect buffers");
        DrawCommandsSystem->UpdateGPUData(pCommandList);
        PROFILE_END(pCommandList);
        GetDevice().ClearUploadListReference();
    }
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
      RenderTargetsSystem->SaveDepthStencilBuffer(&dsvHandle);
      RenderTargetsSystem->SaveDisplayBuffer(&rtvHandle, pBackBuffer);

      ID3D12DescriptorHeap* pDescriptorHeaps = GetDevice().GetDescriptorHeap();
      pCommandList->SetDescriptorHeaps(1, &pDescriptorHeaps);

      RenderTargetsSystem->Set(pCommandList, render_targets_enum::target_display);

      for (int i = 0; i < Passes.size(); i++)
      {
        Passes[i]->DeviceTimeCounter.Start(pCommandList);
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
        Passes[i]->DeviceTimeCounter.Stop(pCommandList);
      }
      DeviceFrameCounter.Stop(pCommandList);
      Device.TransitResourceState(pCommandList, pBackBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
    }
    PROFILE_END(pCommandList);

    Device.CloseSubmitAndPresentRenderCommandList(false);
    auto renderEnd = std::chrono::system_clock::now();
    CPUDrawFrameTime = std::chrono::duration_cast<std::chrono::nanoseconds>(renderEnd - renderStart).count();
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

  {
      delete GlobalsSystem;
      delete RenderTargetsSystem;
      delete ObjectTransformsSystem;
      delete NodeTransformsSystem;
      delete DrawCommandsSystem;
      delete GeometrySystem;
      delete MaterialsSystem;
  }

  for (auto& pass : Passes)
    delete pass;
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