#ifndef __mth_h_
#define __mth_h_

//#define USE_DXMATH
#ifdef USE_DXMATH
#include "mth_def.h"
#include "mth_vec.h"
#include "mth_quat.h"
#include "mth_dxmath_matr.h"
#include "mth_dxmath_operators.h"
#include "mth_dxmath_cam.h"

// some specific types
namespace mth
{
  using vec3f = vec3<float>;
  using vec2f = vec2<float>;
  using vec4f = vec4<float>;
  using matr4f = dxmath_matr;
  using matr = matr4f;
  using cam = cam3;
}
#else
#include "mth_def.h"
#include "mth_vec.h"
#include "mth_quat.h"
#include "mth_matr.h"
#include "mth_operators.h"
#include "mth_cam.h"

// some specific types
namespace mth
{
  using vec3f = vec3<float>;
  using vec2f = vec2<float>;
  using vec4f = vec4<float>;
  using matr4f = matr4<float>;
  using matr = matr4f;
  using cam = cam3<float>;
}
#endif // USE_DXMATH

#endif /* __mth_h_*/

