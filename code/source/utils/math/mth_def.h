#ifndef __mth_def_h_
#define __mth_def_h_

#include <cmath>
#include <def.h>

#ifdef dot
#undef dot
#endif /* dot */
#define dot &

#ifdef cross
#undef cross
#endif /* cross */
#define cross %

/***
 * Useful constants
 ***/

#define MTH_E     2.71828182845904523536f
#define MTH_PI    3.14159265358979323846f
#define MTH_SQRT2 1.41421356237309504880f
#define MTH_D2R   0.01745329251994329576f
#define MTH_R2D   57.2957795130823208767f


//#define clamp(s, in, ax) ((s) < (in) ? (in) : (s) > (ax) ? (ax) : (s))

/* Math namespace */
namespace mth
{
  typedef FLOAT FLT;
  typedef DOUBLE DBL;
  static DBL Degree2Radian = MTH_D2R;
} /* End of 'mth' namespace */

#endif /* __mth_def_h_ */
