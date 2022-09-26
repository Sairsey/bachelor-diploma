#pragma once
#include "unit_base.h"

class unit_stats : public gdr::unit_base
{
private:

  // Transform tool
  bool TransformToolActive;
  int CurrentTransformToShow;
  
  // Material tool
  bool MaterialToolActive;
  int CurrentMaterialToShow;

  // Light tool
  bool LightToolActive;
  int CurrentLightToShow;

  // Stats window
  bool StatsToolActive;
  const int PlotWindowWidth = 10;
  int CurrentPlotIndex = 0;
  std::vector<float> CPURenderTimePlot;
  float AverageCPURenderTimePlot;

  std::vector<float> DeviceRenderTimePlot;
  float AverageDeviceRenderTimePlot;

  std::vector<float> GPUMemoryUsagePlot;
  float AverageGPUMemoryUsagePlot;
  float MaxGPUMemoryPlot;

  gdr::gdr_object PointLightObject;
  gdr::gdr_object DirLightObject;
  gdr::gdr_object SpotLightObject;

  int NullTransform;

public:
  void Initialize(void);

  void Response(void);

  std::string GetName(void)
  {
    return "unit_stats";
  }

  ~unit_stats(void)
  {
  }
};