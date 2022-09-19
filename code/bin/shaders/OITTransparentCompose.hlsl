#include "shared_structures.h"
#include "RandomGenerator.h"
#include "Matrices.h"

cbuffer GlobalValues : register (b0)
{
  GlobalData globals;
}

StructuredBuffer<OITList> OITHeads : register(t5);                       // SRV: Data with Order Independent transparency lists
StructuredBuffer<OITNode> OITPool : register(t6);                        // SRV: Data with Order Independent transparency nodes

struct VSIn
{
  float3 pos : POSITION;
};

struct VSOut
{
  float4 pos : SV_POSITION;
};

VSOut VS(VSIn input)
{ 
    VSOut output;

    output.pos = float4(input.pos, 1.0);
    return output;
}

float4 PS(VSOut input) : SV_TARGET
{
    uint2 screen_pos = uint2(input.pos.x - 0.5, input.pos.y - 0.5);
    float3 color = float3(0, 0, 0);
    OITNode frags[MAX_TRANSPARENT_ARRAY_SIZE];
    uint arraySize = 0;
    uint n = OITHeads[screen_pos.y * globals.width + screen_pos.x].RootIndex;
    while (n != 0xFFFFFFFF && arraySize < MAX_TRANSPARENT_ARRAY_SIZE)
    {
      frags[arraySize] = OITPool[n];
      n = frags[arraySize].NextNodeIndex;
      arraySize++;
    }

    if (arraySize == 0)
      return float4(0, 0, 0, 0);

    uint sorted_amount = arraySize;

    bool merge_sort = true;
    bool insertion_sort = false;
    bool smart_selection_sort = false;
    
    if (merge_sort)
    {
      // sort the array by depth using merge sort (farthest is 0th, nearest is (array_size - 1)th)
      int i, j1, j2, k;
      int a, b, c;
      int step = 1;
      OITNode leftArray[MAX_TRANSPARENT_ARRAY_SIZE / 2]; //for merge sort

      while (step <= arraySize)
      {
        i = 0;
        while (i < arraySize - step)
        {
          ////////////////////////////////////////////////////////////////////////
          //merge(step, i, i + step, min(i + step + step, count));
          a = i;
          b = i + step;
          c = (i + step + step) >= arraySize ? arraySize : (i + step + step);

          for (k = 0; k < step; k++)
            leftArray[k] = frags[a + k];

          j1 = 0;
          j2 = 0;
          for (k = a; k < c; k++)
          {
            if (b + j1 >= c || (j2 < step && leftArray[j2].Depth > frags[b + j1].Depth))
              frags[k] = leftArray[j2++];
            else
              frags[k] = frags[b + j1++];
          }
          ////////////////////////////////////////////////////////////////////////
          i += 2 * step;
        }
        step *= 2;
      }
    }
    else if (insertion_sort)
    {
      for (uint i = 1; i < sorted_amount; i++)
      {
        OITNode toInsert = frags[i];
        uint j = i;
        while (j > 0 && toInsert.Depth > frags[j - 1].Depth) {
          frags[j] = frags[j - 1];
          j--;
        }
        frags[j] = toInsert;
      }
    }
    else if (smart_selection_sort)// selection sort but only MAX_TRANSPARENT_DEPTH elements
    {
      if (sorted_amount > MAX_TRANSPARENT_DEPTH)
        sorted_amount = MAX_TRANSPARENT_DEPTH;

      for (uint i = 0; i < sorted_amount; i++)
      {
        uint min = arraySize - 1;
        for (uint j = arraySize - 1; j > i; j--)
          if (frags[min].Depth > frags[j].Depth)
            min = j;

        // swap i and min
        if (frags[min].Depth < frags[i].Depth)
        {
          OITNode tmp = frags[i];
          frags[i] = frags[min];
          frags[min] = tmp;
        }
      }
      // now 0 is the smallest by depth value and that means it is nearest

      // reverse, so we start from farthest in sorted_amount
      for (uint i = 0; i < sorted_amount / 2; i++)
      {
        OITNode tmp = frags[i];
        frags[i] = frags[sorted_amount - i - 1];
        frags[sorted_amount - i - 1] = tmp;
      }
    }
    else // nothing
    {

    }
    

    float out_alpha = (1.0 - frags[0].Color.a);
    color = frags[0].Color.rgb;
    
    for (int i = 1; i < sorted_amount; i++)
    {
      color = color * (1 - frags[i].Color.a) + frags[i].Color.a * frags[i].Color.rgb;
      out_alpha *= (1 - frags[i].Color.a);
    }

    return float4(color, 1.0 - out_alpha);
}