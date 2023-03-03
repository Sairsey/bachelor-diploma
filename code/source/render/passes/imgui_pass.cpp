#include "p_header.h"

#include "imgui/imgui.h"
#include "imgui/imgui_impl_win32.h"
#include "imgui/imgui_impl_dx12.h"
#include "imgui/imnodes.h"

void gdr::imgui_pass::Initialize(void)
{
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO& io = ImGui::GetIO();

  io.ConfigFlags |= ImGuiConfigFlags_::ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
  io.ConfigFlags |= ImGuiConfigFlags_::ImGuiConfigFlags_DockingEnable;           // Enable Docking
  io.ConfigFlags |= ImGuiConfigFlags_::ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows

  ImNodes::CreateContext();

  // Setup style
  ImGui::StyleColorsDark();
  ImNodes::StyleColorsDark();

  ImGuiStyle& style = ImGui::GetStyle();
  if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
  {
    style.WindowRounding = 0.0f;
    style.Colors[ImGuiCol_WindowBg].w = 1.0f;
  }

  Render->GetDevice().AllocateStaticDescriptors(1, CPUDescriptor, GPUDescriptor);

  ImGui_ImplWin32_Init(Render->GetEngine()->hWnd);
  ImGui_ImplDX12_Init(Render->GetDevice().GetDXDevice(), 3,
    DXGI_FORMAT_R8G8B8A8_UNORM, Render->GetDevice().GetDescriptorHeap(),
      CPUDescriptor, GPUDescriptor);

}

void gdr::imgui_pass::CallDirectDraw(ID3D12GraphicsCommandList* currentCommandList)
{
  Render->RenderTargetsSystem->Set(currentCommandList, render_targets_enum::target_display);
  ImGui_ImplDX12_NewFrame();
  ImGui_ImplWin32_NewFrame();
  ImGui::NewFrame();

  for (auto &lambda : lambdas_to_draw)
    lambda();

  lambdas_to_draw.clear();

  // Rendering
  ImGui::Render();
  ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), currentCommandList);
}

gdr::imgui_pass::~imgui_pass(void)
{
  ImNodes::DestroyContext();
  ImGui_ImplDX12_Shutdown();
  ImGui_ImplWin32_Shutdown();
  ImGui::DestroyContext();
}
