#pragma once
#include "def.h"

namespace gdr
{
  class raycast_manager
  {
    private:
      engine* Engine;

      ray_intersect RayVsOBB(mth::vec3f Org, mth::vec3f Dir, mth::vec3f MinAABB, mth::vec3f MaxAABB, mth::matr4f Transform) const;

    public:

      enum RaycastLayersMask
      {
        LIGHTS = 0x1,
        MODELS = 0x2,

        ALL = 0x3
      };

      raycast_manager(engine* Eng);

      bool Raycast(mth::vec3f Org, mth::vec3f Dir, float MaxLength, RaycastLayersMask layers, std::vector<ray_intersect>* Output = nullptr);
  };

}