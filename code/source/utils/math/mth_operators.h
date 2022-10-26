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

  /* Input vector function
   * ARGUMENTS:
   *   - input stream:
   *       std::istream &In;
   *   - vector to input:
   *       vec4 &Vec;
   * RETURNS:
   *   (std::istream &) link to input stream.
   */
  template<class Type>
    inline std::istream & operator>>( std::istream &In, vec4<Type> &Vec )
    {
      In >> Vec.X >> Vec.Y >> Vec.Z >> Vec.W;
      return In;
    } /* End of 'operator>>' function */

  /* Output vector function
   * ARGUMENTS:
   *   - output stream:
   *       std::ostream &Out;
   *   - vector to output:
   *       const vec4 &Vec;
   * RETURNS:
   *   (std::ostream &) link to output stream.
   */
  template<class Type>
    inline std::ostream & operator<<( std::ostream &Out, const vec4<Type> &Vec )
    {
      Out << Vec.X << " " << Vec.Y << " " << Vec.Z << " " << Vec.W;
      return Out;
    } /* End of 'operator<<' function */

  /* Input vector function
   * ARGUMENTS:
   *   - input stream:
   *       std::istream &In;
   *   - vector to input:
   *       vec3 &Vec;
   * RETURNS:
   *   (std::istream &) link to input stream.
   */
  template<class Type>
    inline std::istream & operator>>( std::istream &In, vec3<Type> &Vec )
    {
      In >> Vec.X >> Vec.Y >> Vec.Z;
      return In;
    } /* End of 'operator>>' function */

  /* Output vector function
   * ARGUMENTS:
   *   - output stream:
   *       std::ostream &Out;
   *   - vector to output:
   *       const vec3 &Vec;
   * RETURNS:
   *   (std::ostream &) link to output stream.
   */
  template<class Type>
    inline std::ostream & operator<<( std::ostream &Out, const vec3<Type> &Vec )
    {
      Out << Vec.X << " " << Vec.Y << " " << Vec.Z;
      return Out;
    } /* End of 'operator<<' function */

  /* Input vector function
   * ARGUMENTS:
   *   - input stream:
   *       std::istream &In;
   *   - vector to input:
   *       vec2 &Vec;
   * RETURNS:
   *   (std::istream &) link to input stream.
   */
  template<class Type>
    inline std::istream & operator>>( std::istream &In, vec2<Type> &Vec )
    {
      In >> Vec.X >> Vec.Y;
      return In;
    } /* End of 'operator>>' function */

  /* Output vector function
   * ARGUMENTS:
   *   - output stream:
   *       std::ostream &Out;
   *   - vector to output:
   *       const vec2 &Vec;
   * RETURNS:
   *   (std::ostream &) link to output stream.
   */
  template<class Type>
    inline std::ostream & operator<<( std::ostream &Out, const vec2<Type> &Vec )
    {
      Out << Vec.X << " " << Vec.Y;
      return Out;
    } /* End of 'operator<<' function */
} /* End of 'mth' namespace */

#endif /* __mth_operators_h_ */
