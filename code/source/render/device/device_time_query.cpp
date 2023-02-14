#pragma once
#include "p_header.h"


gdr::device_time_query::device_time_query(device* pDevice)
  : StartTicks(0)
  , EndTicks(0)
  , Device(pDevice)
  , Freq(0)
{
  if (pDevice != nullptr)
  {
    Freq = pDevice->GetPresentQueueFrequency();
  }
}

void gdr::device_time_query::Start(ID3D12GraphicsCommandList* pCmdList)
{
  GDR_ASSERT(Device != nullptr);
  Device->QueryTimestamp(pCmdList, [&start = StartTicks](UINT64 value) {start = value; });
}

void gdr::device_time_query::Stop(ID3D12GraphicsCommandList* pCmdList)
{
  GDR_ASSERT(Device != nullptr);
  Device->QueryTimestamp(pCmdList, [&end = EndTicks](UINT64 value) {end = value; });
}

/** Get value in milliseconds */
double gdr::device_time_query::GetMSec() const
{
  return (double)(EndTicks - StartTicks) / Freq * 1000.0;
}

/** Get value in milliseconds */
double gdr::device_time_query::GetUSec() const
{
  return (double)(EndTicks - StartTicks) / Freq * 1000000.0;
}