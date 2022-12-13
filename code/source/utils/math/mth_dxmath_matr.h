#ifndef __mth_dxmath_matr_h_
#define __mth_dxmath_matr_h_

#include "DirectXMath/DirectXMath.h"

/* Math namespace */
namespace mth
{
  /* 4x4 matrix class */
  class dxmath_matr
  {
  public:
    DirectX::XMMATRIX data;
    /* Constructor
     * ARGUMENTS:
     *   - matrix components:
     *       float M11, M12, M13, M14,
     *            M21, M22, M23, M24,
     *            M31, M32, M33, M34,
     *            M41, M42, M43, M44;
     */
    inline dxmath_matr(float M11, float M12, float M13, float M14,
      float M21, float M22, float M23, float M24,
      float M31, float M32, float M33, float M34,
      float M41, float M42, float M43, float M44) : data(M11, M12, M13, M14, M21, M22, M23, M24, M31, M32, M33, M34, M41, M42, M43, M44)
    {
    } /* End of 'dxmath_matr' function */

    /* Default constuctor
     * ARGUMENTS:
     *   - identical value:
     *       float V = 0;
     */
    inline explicit dxmath_matr(float V = 0) : data(V, V, V, V, V, V, V, V, V, V, V, V, V, V, V, V)
    {
    } /* End of 'dxmath_matr<float>' function */

    /* Default constuctor
     * ARGUMENTS:
     *   - identical value:
     *       float V = 0;
     */
    inline explicit dxmath_matr(const DirectX::XMMATRIX &A) : data(A)
    {
    } /* End of 'dxmath_matr<float>' function */

    /* Rotation matrix from quaternion function.
     * ARGUMENTS:
     *   - quaternion:
     *       const quat<float> Quat;
     */
    inline dxmath_matr(const quat<float>& Q) : data(DirectX::XMMatrixRotationQuaternion({ Q.Q1, Q.Q2, Q.Q3, Q.Q4 }))
    {
    } /* End of 'dxmath_matr<float>

    /* Pointer to array constuctor
     * ARGUMENTS:
     *   - pointer to 4x4 array:
     *       float *V;
     */
    inline dxmath_matr(float* V) : data(V)
    {
    } /* End of 'dxmath_matr<float>' function */

    /* Identity matrix generator
     * ARGUMENTS: None.
     * RETURNS: (dxmath_matr<float>) identity matrix
     */
    inline static dxmath_matr Identity(VOID)
    {
      return dxmath_matr(DirectX::XMMatrixIdentity());
    } /* End of 'Identity' funciton */

    /* matrix generator from quaternion And position
     * ARGUMENTS: None.
     * RETURNS: (dxmath_matr<float>) matrix
     */
    inline static dxmath_matr FromQuaternionAndPosition(vec4<float> quat, vec3<float> pos)
    {
      return dxmath_matr(
        1 - 2 * quat.Y * quat.Y - 2 * quat.Z * quat.Z, 2 * quat.X * quat.Y - 2 * quat.Z * quat.W, 2 * quat.X * quat.Z + 2 * quat.Y * quat.W, 0,
        2 * quat.X * quat.Y + 2 * quat.Z * quat.W, 1 - 2 * quat.X * quat.X - 2 * quat.Z * quat.Z, 2 * quat.Z * quat.Y - 2 * quat.X * quat.W, 0,
        2 * quat.X * quat.Z - 2 * quat.Y * quat.W, 2 * quat.Z * quat.Y + 2 * quat.X * quat.W, 1 - 2 * quat.X * quat.X - 2 * quat.Y * quat.Y, 0,
        pos.X, pos.Y, pos.Z, 1);
    }

    /* Rotation around X axis transformation matrix setup function.
     * ARGUMENTS:
     *   - rotation angle in degrees:
     *       float AngleInDegree;
     * RETURNS:
     *   (dxmath_matr<float>) result matrix.
     */
    inline static dxmath_matr RotateX(float AngleInDegree)
    {
      AngleInDegree *= MTH_D2R;
      return dxmath_matr(DirectX::XMMatrixRotationX(AngleInDegree));
    } /* End of 'RotateX' funciton */

    /* Rotation around Y axis transformation matrix setup function.
     * ARGUMENTS:
     *   - rotation angle in degrees:
     *       float AngleInDegree;
     * RETURNS:
     *   (dxmath_matr<float>) result matrix.
     */
    inline static dxmath_matr RotateY(float AngleInDegree)
    {
      AngleInDegree *= MTH_D2R;
      return dxmath_matr(DirectX::XMMatrixRotationY(AngleInDegree));
    } /* End of 'RotateY' funciton */

    /* Rotation around Z axis transformation matrix setup function.
     * ARGUMENTS:
     *   - rotation angle in degrees:
     *       float AngleInDegree;
     * RETURNS:
     *   (dxmath_matr<float>) result matrix.
     */
    inline static dxmath_matr RotateZ(float AngleInDegree)
    {
      AngleInDegree *= MTH_D2R;
      return dxmath_matr(DirectX::XMMatrixRotationZ(AngleInDegree));
    } /* End of 'RotateZ' funciton */

    /* Decompose matrix to its elements.
     * ARGUMENTS:
     * RETURNS:
     *   (vec3<float>) Rotation per each axis in degrees.
     */
    vec3<float> GetEulerAngles()
    {
      vec3<float> Ans;
      float sy = sqrt(data.r[0].m128_f32[0] * data.r[0].m128_f32[0] + data.r[1].m128_f32[0] * data.r[1].m128_f32[0]);
      bool is_singular = sy < 0.00005;

      if (!is_singular)
      {
        Ans.X = atan2(data.r[2].m128_f32[1], data.r[2].m128_f32[2]);
        Ans.Y = atan2(-data.r[2].m128_f32[0], sy);
        Ans.Z = atan2(data.r[1].m128_f32[0], data.r[0].m128_f32[0]);
      }
      else
      {
        Ans.X = atan2(-data.r[1].m128_f32[2], data.r[1].m128_f32[1]);
        Ans.Y = atan2(-data.r[2].m128_f32[0], sy);
        Ans.Z = 0;
      }

      Ans.X = Ans.X < 0 ? Ans.X + 2 * MTH_PI : Ans.X;
      Ans.Y = Ans.Y < 0 ? Ans.Y + 2 * MTH_PI : Ans.Y;
      Ans.Z = Ans.Z < 0 ? Ans.Z + 2 * MTH_PI : Ans.Z;

      return Ans * MTH_R2D;
    }

    /* Decompose matrix to its elements.
     * ARGUMENTS:
     * RETURNS:
     *   (vec3<float>) Translation vector.
     *   (vec3<float>) Rotation per each axis in degrees.
     *   (vec3<float>) Scale per each axis.
     */
    void Decompose(vec3<float>& Translation, vec3<float>& RotationPerAxis, vec3<float>& Scale)
    {
      DirectX::XMVECTOR Trans, Rot, Scl;
      DirectX::XMMatrixDecompose(&Scl, &Rot, &Trans, data);
      Translation.X = DirectX::XMVectorGetX(Trans);
      Translation.Y = DirectX::XMVectorGetY(Trans);
      Translation.Z = DirectX::XMVectorGetZ(Trans);

      Scale.X = DirectX::XMVectorGetX(Scl);
      Scale.Y = DirectX::XMVectorGetY(Scl);
      Scale.Z = DirectX::XMVectorGetZ(Scl);

      DirectX::XMFLOAT4X4 XMFLOAT4X4_Values;
      DirectX::XMStoreFloat4x4(&XMFLOAT4X4_Values, DirectX::XMMatrixTranspose(data));
      RotationPerAxis.X = (float)asin(-XMFLOAT4X4_Values._23);
      RotationPerAxis.Y = (float)atan2(XMFLOAT4X4_Values._13, XMFLOAT4X4_Values._33);
      RotationPerAxis.Z = (float)atan2(XMFLOAT4X4_Values._21, XMFLOAT4X4_Values._22);
    }

    /* Translation matrix setup function.
     * ARGUMENTS:
     *   - translation vector:
     *       const vec3 &Vec;
     * RETURNS:
     *   (dxmath_matr<float>) result translation matrix.
     */
    inline static dxmath_matr Translate(const vec3<float>& Vec)
    {
      return dxmath_matr(DirectX::XMMatrixTranslation(Vec.X, Vec.Y, Vec.Z));
    } /* End of 'Translate' function */

    /* Scale transformation matrix initializing function .
     * ARGUMENTS:
     *   - scale vector:
     *       const vec3 &Vec;
     * RETURNS:
     *   (vec) scale transformation initialized matrix structure.
     */
    inline static dxmath_matr Scale(const vec3<float>& Vec)
    {
      return dxmath_matr(DirectX::XMMatrixScaling(Vec.X, Vec.Y, Vec.Z));
    } /* End of 'Scale' funciton */

    /* Rotation around arbitraty axis function.
     * ARGUMENTS:
     *   - angle in degrees:
     *       float AngleInDegree;
     *   - axis:
     *       const vec3 &Axis;
     * RETURNS:
     *   (MATR) result rotation matrix.
     */
    inline static dxmath_matr Rotate(float AngleInDegree, const vec3<float>& Axis)
    {
      return dxmath_matr(DirectX::XMMatrixRotationAxis({Axis.X, Axis.Y, Axis.Z}, AngleInDegree * MTH_D2R));
    } /* End of 'Rotate' function */

    /* View transformation matrix initializing function .
     * ARGUMENTS:
     *   - view transformation parametrs:
     *      vec Loc, At, Up;
     * RETURNS:
     *   (dxmath_matr<float>) view transformation initialized matrix structure.
     */
    inline static dxmath_matr View(vec3<float> Loc, vec3<float> At, vec3<float> Up)
    {
      return dxmath_matr(DirectX::XMMatrixLookAtLH(
        {Loc.X, Loc.Y, Loc.Z},
        {At.X, At.Y, At.Z },
        {Up.X, Up.Y, Up.Z}));
    } /* End of 'View' function */

    /* Orthographic transformation matrix initializing function .
     * ARGUMENTS:
     *   - orthographic transformation matrix parametrs:
     *       float
     *         Left, Right,
     *         Bottom, Top,
     *         Near,  Far;
     * RETURNS:
     *   (dxmath_matr<float>) orthographic transformation initialized matrix structure.
     */
    inline static dxmath_matr Ortho(float Left, float Right, float Bottom, float Top, float Near, float Far)
    {
      return dxmath_matr(DirectX::XMMatrixOrthographicLH(Right-Left, Bottom - Top, Near, Far));
    } /* End of 'Ortho' function */

    /* Frustum transformation matrix initializing function.
     * ARGUMENTS:
     *   - frustum transformation matrix parametrs:
     *       float
     *         Left, Right,
     *         Bottom, Top,
     *         Near,  Far;
     * RETURNS:
     *   (dxmath_matr<float>) frustum transformation initialized matrix structure.
     */
    inline static dxmath_matr Frustum(float Left, float Right, float Bottom, float Top, float Near, float Far)
    {
      return dxmath_matr(DirectX::XMMatrixPerspectiveLH(Right - Left, Bottom - Top, Near, Far));
    } /* End of 'Frustum' function */

    /* Get matrix line funciton.
     * ARGUMENTS:
     *   - index of line:
     *       UINT Index;
     * RETURNS:
     *   (float *) matrix line.
     */
    inline float* operator[](UINT Index)
    {
      return (float *)(&data.r[Index]);
    } /* End of 'operator[]' funciton */

    /* Get constant matrix line funciton.
     * ARGUMENTS:
     *   - index of line:
     *       UINT Index;
     * RETURNS:
     *   (float *) matrix line.
     */
    inline const float* operator[](UINT Index) const
    {
      return (float*)(&data.r[Index]);
    } /* End of 'operator[]' funciton */

    /* Matrix multiplication funciton.
     * ARGUMENTS:
     *   - matrix to be multiplated to:
     *       const dxmath_matr<float> &Matr;
     * RETURNS:
     *   (dxmath_matr<float>) result matrix.
     */
    inline dxmath_matr operator*(const dxmath_matr& Matr) const
    {
      return dxmath_matr(DirectX::XMMatrixMultiply(data, Matr.data));
    } /* End of 'operator*' fucntion */

    /* Matrix multiplication funciton.
     * ARGUMENTS:
     *   - matrix to be multiplated to:
     *       const dxmath_matr<float> &Matr;
     * RETURNS:
     *   (dxmath_matr<float> &) link to this matrix.
     */
    inline dxmath_matr& operator*=(const dxmath_matr& Matr)
    {
      data = DirectX::XMMatrixMultiply(data, Matr.data);
      return *this;
    } /* End of 'operator*=' fucntion */

    /* Matrix determinant funcion
     * ARGUMENTS: None.
     * RETURNS:
     *   (float) determinant.
     */
    inline float Determ(VOID) const
    {
      return DirectX::XMVectorGetX(DirectX::XMMatrixDeterminant(data));
    } /* End of 'Determ' function */

    /* Matrix invertion funcion
     * ARGUMENTS: None.
     * RETURNS:
     *   (dxmath_matr<float>) inversed matrix.
     */
    inline dxmath_matr Inversed(VOID) const
    {
      DirectX::XMVECTOR vec = DirectX::XMMatrixDeterminant(data);
      return dxmath_matr(DirectX::XMMatrixInverse(&vec, data));
    } /* End of 'Inversed' function */

    /* Current matrix invertion funcion
     * ARGUMENTS: None.
     * RETURNS:
     *   (dxmath_matr<float> &) link to this matrix.
     */
    inline dxmath_matr& Inverse(VOID)
    {
      DirectX::XMVECTOR vec = DirectX::XMMatrixDeterminant(data);
      data = DirectX::XMMatrixInverse(&vec, data);
      return *this;
    } /* End of 'Inverse' function */

    /* Current matrix transpose function.
     * ARGUMENTS: None.
     * RETURNS:
     *   (dxmath_matr<float> &) link to this matrix.
     */
    inline dxmath_matr& Transpose(VOID)
    {
      data = DirectX::XMMatrixTranspose(data);
      return *this;
    } /* End of 'Transpose' function */

    /* Matrix transpose function.
     * ARGUMENTS: None.
     * RETURNS:
     *   (dxmath_matr<float>) transposed matrix.
     */
    inline dxmath_matr Transposed(VOID) const
    {
      return dxmath_matr(DirectX::XMMatrixTranspose(data));
    } /* End of 'Transposed' function */

    /* Matrix equality funciton.
     * ARGUMENTS:
     *   - matrix to be comparised with:
     *       const dxmath_matr<float> &Matr;
     * RETURNS:
     *   (BOOL) who of them is Bazhen??.
     */
    inline BOOL operator==(const dxmath_matr& Matr) const
    {
      return memcmp(&Matr.data, &data, sizeof(data)) == 0;
    } /* End of 'operator==' function */
  }; /* End of 'dxmath_matr' class */
} /* End of 'mth' namespace */

#endif /* __mth_dxmath_matr_h_ */
