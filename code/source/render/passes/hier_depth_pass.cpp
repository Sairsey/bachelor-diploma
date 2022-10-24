#include "p_header.h"

void gdr::hier_depth_pass::Initialize(void)
{
}

void gdr::hier_depth_pass::CallDirectDraw(ID3D12GraphicsCommandList* currentCommandList)
{
  if (!Render->Params.IsVisibilityLocked)
    Render->HierDepth->Generate(currentCommandList);
}

gdr::hier_depth_pass::~hier_depth_pass(void)
{
}
