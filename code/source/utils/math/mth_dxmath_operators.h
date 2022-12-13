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
  inline vec3<Type> operator*(const vec3<Type>& Vec, const dxmath_matr& Matr)
  {
    DirectX::XMFLOAT3 v;
    DirectX::XMStoreFloat3(&v, DirectX::XMVector3Transform(DirectX::XMVectorSet(Vec.X, Vec.Y, Vec.Z, 1), Matr.data));

    return {v.x, v.y, v.z};
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
  inline vec3<Type> operator*(const dxmath_matr& Matr, const vec3<Type>& Vec)
  {
    DirectX::XMFLOAT3 v;
    DirectX::XMStoreFloat3(&v, DirectX::XMVector3Transform(DirectX::XMVectorSet(Vec.X, Vec.Y, Vec.Z, 1), Matr.data));

    return { v.x, v.y, v.z };
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
  inline vec3<Type>& operator*=(vec3<Type>& Vec, const dxmath_matr& Matr)
  {
    DirectX::XMFLOAT3 v;
    DirectX::XMStoreFloat3(&v, DirectX::XMVector3Transform(DirectX::XMVectorSet(Vec.X, Vec.Y, Vec.Z, 1), Matr.data));
    Vec = { v.x, v.y, v.z };
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
  inline std::istream& operator>>(std::istream& In, vec4<Type>& Vec)
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
  inline std::ostream& operator<<(std::ostream& Out, const vec4<Type>& Vec)
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
  inline std::istream& operator>>(std::istream& In, vec3<Type>& Vec)
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
  inline std::ostream& operator<<(std::ostream& Out, const vec3<Type>& Vec)
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
  inline std::istream& operator>>(std::istream& In, vec2<Type>& Vec)
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
  inline std::ostream& operator<<(std::ostream& Out, const vec2<Type>& Vec)
  {
    Out << Vec.X << " " << Vec.Y;
    return Out;
  } /* End of 'operator<<' function */
} /* End of 'mth' namespace */

#endif /* __mth_operators_h_ */
