#pragma once
#include "def.h"

/* Project namespace */
namespace gdr
{
  // TODO Move it to some more appropriate place
  class device_time_query
  {
  public:
    device_time_query(device* pDevice = nullptr);
  
    void Start(ID3D12GraphicsCommandList* pCmdList);
    void Stop(ID3D12GraphicsCommandList* pCmdList);
  
    // Get value in milliseconds
    double GetMSec() const;
    // Get value in microseconds
    double GetUSec() const;
  
  private:
    UINT64 StartTicks;
    UINT64 EndTicks;
    UINT64 Freq;
  
    device* Device;
  };
}