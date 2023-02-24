#ifndef __mth_operators_h_
#define __mth_operators_h_

/* Math namespace */
namespace mth
{
  /* Multiplication 3 components vector by 4x4 matrix
   * with substract by caculated homogenious coordinate.
   * ARGUMENTS:
   *   - vector:
   *       const vec3<Type> &Vec;
   *   - matrix:
   *       const matr4<Type> &Matr;
   * RETURNS:
   *   (vec3) vector multipication vector on matrix.
   */
  template<class Type>
    inline vec3<Type> operator*( const vec3<Type> &Vec, const matr4<Type> &Matr )
    {
      Type w = Vec.X * Matr[0][3] + Vec.Y * Matr[1][3] + Vec.Z * Matr[2][3] + Matr[3][3];

      return vec3<Type>(Vec.X * Matr[0][0] + Vec.Y * Matr[1][0] + Vec.Z * Matr[2][0] + Matr[3][0],
                        Vec.X * Matr[0][1] + Vec.Y * Matr[1][1] + Vec.Z * Matr[2][1] + Matr[3][1],
                        Vec.X * Matr[0][2] + Vec.Y * Matr[1][2] + Vec.Z * Matr[2][2] + Matr[3][2]) / w;
    } /* End of 'operator*' funciton */

  /* Multiplication 3 components vector by 4x4 matrix
   * with substract by caculated homogenious coordinate.
   * ARGUMENTS:
   *   - matrix:
   *       const matr4 &Matr;
   *   - vector:
   *       const vec3 &Vec;
   * RETURNS:
   *   (vec3) vector multipication vector on matrix.
   */
  template<class Type>
    inline vec3<Type> operator*( const matr4<Type> &Matr, const vec3<Type> &Vec )
    {
      Type w = Vec.X * Matr[0][3] + Vec.Y * Matr[1][3] + Vec.Z * Matr[2][3] + Matr[3][3];

      return vec3<Type>(Vec.X * Matr[0][0] + Vec.Y * Matr[1][0] + Vec.Z * Matr[2][0] + Matr[3][0],
                        Vec.X * Matr[0][1] + Vec.Y * Matr[1][1] + Vec.Z * Matr[2][1] + Matr[3][1],
                        Vec.X * Matr[0][2] + Vec.Y * Matr[1][2] + Vec.Z * Matr[2][2] + Matr[3][2]) / w;
    } /* End of 'operator*' funciton */

  /* Multiplication 3 components vector by 4x4 matrix
   * with substract by caculated homogenious coordinate.
   * ARGUMENTS:
   *   - vector:
   *       const vec3 &Vec;
   *   - matrix:
   *       const matr4 &Matr;
   * RETURNS:
   *   (vec3 &) link to Vec.
   */
  template<class Type>
    inline vec3<Type> & operator*=( vec3<Type> &Vec, const matr4<Type> &Matr )
    {
      Type w = Vec.X * Matr[3][0] + Vec.Y * Matr[3][1] + Vec.Z * Matr[3][2] + Matr[3][3];

      Vec = vec3<Type>(Vec.X * Matr[0][0] + Vec.Y * Matr[0][1] + Vec.Z * Matr[0][2] + Matr[0][3],
                       Vec.X * Matr[1][0] + Vec.Y * Matr[1][1] + Vec.Z * Matr[1][2] + Matr[1][3],
                       Vec.X * Matr[2][0] + Vec.Y * Matr[2][1] + Vec.Z * Matr[2][2] + Matr[2][3]) / w;
      return Vec;
    } /* End of 'operator*=' funciton */
} /* End of 'mth' namespace */

#endif /* __mth_operators_h_ */
