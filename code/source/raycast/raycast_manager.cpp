#include "p_header.h"
#include "raycast_manager.h"

gdr::raycast_manager::raycast_manager(engine* Eng) : Engine(Eng)
{
}


ray_intersect gdr::raycast_manager::RayVsOBB(mth::vec3f Org, mth::vec3f Dir, mth::vec3f MinAABB, mth::vec3f MaxAABB, mth::matr4f Transform) const
{
	ray_intersect A;
	A.Index = NONE_INDEX;

	float tMin = 0.0;
	float tMax = FLT_MAX; //100000.0;
	//const threshold = 0.001;
	const float threshold = 1e-6;

	mth::vec3f pos = { Transform[3][0], Transform[3][1], Transform[3][2] };
	mth::vec3f delta = pos - Org;

	// Test intersection with the 2 planes perpendicular to the OBB's X axis
  for (int axis = 0; axis < 3; axis++)
	{
		mth::vec3f vecaxis = { Transform[axis][0], Transform[axis][1], Transform[axis][2] };

    if (vecaxis == mth::vec3f(0))
      continue;

		float e = vecaxis dot delta;
		float f = Dir dot vecaxis;

		if (std::fabs(f) > threshold) { // Standard case

			float t1 = (e + MinAABB[axis]) / f; // Intersection with the "left" plane
			float t2 = (e + MaxAABB[axis]) / f; // Intersection with the "right" plane
			// t1 and t2 now contain distances betwen ray origin and ray-plane intersections

			// We want t1 to represent the nearest intersection, 
			// so if it's not the case, invert t1 and t2
			if (t1 > t2) {
				float w = t1; 
				t1 = t2; 
				t2 = w; // swap t1 and t2
			}

			// tMax is the nearest "far" intersection (amongst the X,Y and Z planes pairs)
			if (t2 < tMax)
				tMax = t2;
			// tMin is the farthest "near" intersection (amongst the X,Y and Z planes pairs)
			if (t1 > tMin)
				tMin = t1;

			// And here's the trick :
			// If "far" is closer than "near", then there is NO intersection.
			// See the images in the tutorials for the visual explanation.
			if (tMax < tMin)
				return A;
		}
		else { // Rare case : the ray is almost parallel to the planes, so they don't have any "intersection"
			if (-e + MinAABB[axis] > 0.0 || -e + MaxAABB[axis] < 0.0)
				return A;
		}
	}
  
  if (tMin != 0)
  {
    A.Index = 0;
    A.Distance = tMin;
    A.Position = Org + Dir * A.Distance;
  }
  return A;
}

bool gdr::raycast_manager::Raycast(mth::vec3f Org, mth::vec3f Dir, float MaxLength, RaycastLayersMask layers, std::vector<ray_intersect>* Output)
{
  bool status = false;
  if (layers & RaycastLayersMask::LIGHTS)
  {
    for (int i = 0; i < Engine->LightsSystem->AllocatedSize(); i++)
      if (Engine->LightsSystem->IsExist(i) && Engine->ObjectTransformsSystem->IsExist(Engine->LightsSystem->Get(i).ObjectTransformIndex))
      {
        GDRGPUObjectTransform transform = Engine->ObjectTransformsSystem->Get(Engine->LightsSystem->Get(i).ObjectTransformIndex);
        ray_intersect cast = RayVsOBB(Org, Dir, transform.minAABB, transform.maxAABB, transform.Transform);
        if (cast.Index != NONE_INDEX)
        {
          if (Output == nullptr)
            return true;
          else
          {
            cast.Index = i;
            cast.Index.type = gdr_index_types::light;
            Output->push_back(cast);
            status = true;
          }
        }
      }
  }

  if (layers & RaycastLayersMask::MODELS)
  {
    for (int i = 0; i < Engine->ModelsManager->AllocatedSize(); i++)
      if (Engine->ModelsManager->IsExist(i) && Engine->ObjectTransformsSystem->IsExist(Engine->ModelsManager->Get(i).Render.RootTransform))
      {
        GDRGPUObjectTransform transform = Engine->ObjectTransformsSystem->Get(Engine->ModelsManager->Get(i).Render.RootTransform);
        ray_intersect cast = RayVsOBB(Org, Dir, transform.minAABB, transform.maxAABB, transform.Transform);
        if (cast.Index != NONE_INDEX)
        {
          if (Output == nullptr)
            return true;
          else
          {
            cast.Index = i;
            cast.Index.type = gdr_index_types::model;
            Output->push_back(cast);
            status = true;
          }
        }
      }
  }

  if (Output == nullptr)
    return false;

  std::sort(Output->begin(), Output->end(), [](const ray_intersect& a, const ray_intersect& b)
    {
      return a.Distance < b.Distance;
    });
  return status;
}