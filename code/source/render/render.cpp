#include "p_header.h"

// Default constructor
gdr::render::render() : IsInited(true)
{
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

  IsInited = true;
  
  gdr::device_create_params params;
  params.hWnd = Eng->hWnd;

  IsInited = Device.Init(params);

  auto size = Eng->GetSize();

  Viewport.TopLeftX = 0.0f;
  Viewport.TopLeftY = 0.0f;
  Viewport.Height = (FLOAT)size.second;
  Viewport.Width = (FLOAT)size.first;
  Viewport.MinDepth = 0.0f;
  Viewport.MaxDepth = 1.0f;

  Rect.left = 0;
  Rect.top = 0;
  Rect.bottom = (FLOAT)size.second;
  Rect.right = (FLOAT)size.first;

  return IsInited;
}

/* Draw frame function */
void gdr::render::DrawFrame(void)
{
  // temporary draw just plain color

  ID3D12GraphicsCommandList* pCommandList = nullptr;
  ID3D12Resource* pBackBuffer = nullptr;
  D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle;
  if (Device.BeginRenderCommandList(&pCommandList, &pBackBuffer, &rtvHandle))
  {
    HRESULT hr = S_OK;
    pCommandList->OMSetRenderTargets(1, &rtvHandle, TRUE, nullptr);

    pCommandList->RSSetViewports(1, &Viewport);
    pCommandList->RSSetScissorRects(1, &Rect);

    if (Device.TransitResourceState(pCommandList, pBackBuffer, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET))
    {
      FLOAT clearColor[4] = { 0.3f, 0.5f, 0.7f, 1.0f };
      //clearColor[0] = 1.0 * rand() / RAND_MAX;
      //clearColor[1] = 1.0 * rand() / RAND_MAX;
      //clearColor[2] = 1.0 * rand() / RAND_MAX;
      pCommandList->ClearRenderTargetView(rtvHandle, clearColor, 1, &Rect);

      Device.TransitResourceState(pCommandList, pBackBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
    }

    Device.CloseSubmitAndPresentRenderCommandList();
  }

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

  Device.Term();
}

// Default destructor
gdr::render::~render()
{
  Term();
}