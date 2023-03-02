#ifndef __mth_matr_h_
#define __mth_matr_h_

#if _WIN32 && !_WIN64
#define sincos(Angle, SinX, CosX) \
        /* FST(0) Angle (from degree to radian) */ \
        __asm fld  Angle                           \
        __asm fmul Degree2Radian                   \
                                                   \
        /* FST(0) - cos, FST(1) - sin */           \
        __asm fsincos                              \
                                                   \
        __asm fstp CosX /* cos -> cosx */          \
        __asm fstp SinX /* sin -> sinx */
#else
#define sincos(Angle, SinX, CosX) \
        {                         \
          Angle *= MTH_D2R;       \
          SinX = sin(Angle);      \
          CosX = cos(Angle);      \
        }
#endif
/* Math namespace */
namespace mth
{
  /* 4x4 matrix class */
  template<class Type>
    class matr4
    {
    private:
      Type A[4][4]; /* Matrix values */

      /* 3x3 Matrix determinant funcion
       * ARGUMENTS:
       *   - matrix components:
       *       Type M11, M21, M31,
       *            M12, M22, M32,
       *            M13, M23, M33;
       * RETURNS:
       *   (Type) determinant.
       */
      inline static Type Determ3x3( Type M11, Type M21, Type M31,
                                    Type M12, Type M22, Type M32,
                                    Type M13, Type M23, Type M33 )
      {
        return
          M11 * M22 * M33 + 
          M12 * M23 * M31 + 
          M13 * M21 * M32 -
          M11 * M23 * M32 - 
          M12 * M21 * M33 - 
          M13 * M22 * M31;
      } /* End of 'Determ3x3' fucntion */

    public:
      /* Constructor
       * ARGUMENTS:
       *   - matrix components:
       *       Type M11, M12, M13, M14,
       *            M21, M22, M23, M24,
       *            M31, M32, M33, M34,
       *            M41, M42, M43, M44;
       */
      inline matr4<Type>( Type M11, Type M12, Type M13, Type M14,
                          Type M21, Type M22, Type M23, Type M24,
                          Type M31, Type M32, Type M33, Type M34,
                          Type M41, Type M42, Type M43, Type M44)
      {
        A[0][0] = M11, A[1][0] = M21, A[2][0] = M31, A[3][0] = M41;
        A[0][1] = M12, A[1][1] = M22, A[2][1] = M32, A[3][1] = M42;
        A[0][2] = M13, A[1][2] = M23, A[2][2] = M33, A[3][2] = M43;
        A[0][3] = M14, A[1][3] = M24, A[2][3] = M34, A[3][3] = M44;
      } /* End of 'matr4<Type>' function */

      /* Default constuctor
       * ARGUMENTS:
       *   - identical value:
       *       Type V = 0;
       */
      inline explicit matr4<Type>( Type V = 0 )
      {
        for (UINT i = 0; i < 4; i++)
          for (UINT j = 0; j < 4; j++)
            A[i][j] = V;
      } /* End of 'matr4<Type>' function */

      /* Rotation matrix from quaternion function.
       * ARGUMENTS:
       *   - quaternion:
       *       const quat<Type> Quat;
       */
      inline matr4<Type>( const quat<Type> &Q )
      {
        auto sqr = []( const Type &Value )
        { 
          return Value * Value;
        };

        //*this = matr4<Type>{
        //  2 * sqr(Q.Q1) - 1 + 2 * sqr(Q.Q2), 2 * (Q.Q2 * Q.Q3 + Q.Q1 * Q.Q4),   2 * (Q.Q2 * Q.Q4 - Q.Q1 * Q.Q3), 0,
        //    2 * (Q.Q2 * Q.Q3 - Q.Q1 * Q.Q4), 2 * sqr(Q.Q1) - 1 + 2 * sqr(Q.Q3), 2 * (Q.Q3 * Q.Q4 + Q.Q1 * Q.Q2), 0,
        //    2 * (Q.Q2 * Q.Q4 + Q.Q1 * Q.Q3), 2 * (Q.Q3 * Q.Q4 - Q.Q1 * Q.Q2),   2 * sqr(Q.Q1) - 1 + 2 * sqr(Q.Q4), 0,
        //  0, 0, 0, 1
        //};

        *this = matr4{
          1 - 2 * Q.Q3 * Q.Q3 - 2 * Q.Q4 * Q.Q4, 2 * Q.Q2 * Q.Q3 - 2 * Q.Q1 * Q.Q4,     2 * Q.Q2 * Q.Q4 + 2 * Q.Q1 * Q.Q3,     0,
          2 * Q.Q2 * Q.Q3 + 2 * Q.Q1 * Q.Q4,     1 - 2 * Q.Q2 * Q.Q2 - 2 * Q.Q4 * Q.Q4, 2 * Q.Q3 * Q.Q4 - 2 * Q.Q1 * Q.Q2,     0,
          2 * Q.Q2 * Q.Q4 - 2 * Q.Q1 * Q.Q3,     2 * Q.Q3 * Q.Q4 + 2 * Q.Q1 * Q.Q2,     1 - 2 * Q.Q2 * Q.Q2 - 2 * Q.Q3 * Q.Q3, 0, 
          0,                                     0,                                     0,                                     1};
                 
      } /* End of 'matr4<Type>


      /* Pointer to array constuctor
       * ARGUMENTS:
       *   - pointer to 4x4 array:
       *       Type *V;
       */
      inline matr4<Type>( Type *V )
      {
        memcpy(A, V, sizeof(A));
      } /* End of 'matr4<Type>' function */

      /* Identity matrix generator
       * ARGUMENTS: None.
       * RETURNS: (matr4<Type>) identity matrix
       */
      inline static matr4<Type> Identity( VOID )
      {
        return matr4<Type>(1, 0, 0, 0,
                           0, 1, 0, 0,
                           0, 0, 1, 0,
                           0, 0, 0, 1);
      } /* End of 'Identity' funciton */

      /* matrix generator from quaternion And position
       * ARGUMENTS: None.
       * RETURNS: (matr4<Type>) matrix
       */
      inline static matr4<Type> FromQuaternionAndPosition(vec4<Type> quat, vec3<Type> pos)
      {
        return matr4<Type>(
            1 - 2 * quat.Y * quat.Y - 2 * quat.Z * quat.Z, 2 * quat.X * quat.Y - 2 * quat.Z * quat.W, 2 * quat.X * quat.Z + 2 * quat.Y * quat.W, 0,
            2 * quat.X * quat.Y + 2 * quat.Z * quat.W, 1 - 2 * quat.X * quat.X - 2 * quat.Z * quat.Z, 2 * quat.Z * quat.Y - 2 * quat.X * quat.W, 0,
            2 * quat.X * quat.Z - 2 * quat.Y * quat.W, 2 * quat.Z * quat.Y + 2 * quat.X * quat.W, 1 - 2 * quat.X * quat.X - 2 * quat.Y * quat.Y, 0,
                           pos.X, pos.Y, pos.Z, 1);
      }

      /* matrix generator from quaternion And position
       * ARGUMENTS: None.
       * RETURNS: (matr4<Type>) matrix
       */
      inline static matr4<Type> BuildTransform(vec3<Type> scale, vec4<Type> quat, vec3<Type> pos)
      {
        Type angle = 2 * acos(quat.W) * MTH_R2D;
        mth::vec3<Type> axis = {
          quat.X / sqrt(1 - quat.W * quat.W),
          quat.Y / sqrt(1 - quat.W * quat.W),
          quat.Z / sqrt(1 - quat.W * quat.W)
        };
        axis.Normalize();

        if (quat.W == 1)
        {
          axis = { 1, 0, 0 };
          angle = 0;
        }

        return mth::matr4<Type>::Rotate(angle, axis)* mth::matr4<Type>::Scale(scale)* mth::matr4<Type>::Translate(pos);
      }

      /* Rotation around X axis transformation matrix setup function.
       * ARGUMENTS:
       *   - rotation angle in degrees:
       *       Type AngleInDegree;
       * RETURNS:
       *   (matr4<Type>) result matrix.
       */
      inline static matr4<Type> RotateX( Type AngleInDegree )
      {
        Type sinx, cosx;
        sincos(AngleInDegree, sinx, cosx);
        return matr4<Type>(1,    0,     0, 0,
                           0, cosx, -sinx, 0,
                           0, sinx,  cosx, 0,
                           0,    0,     0, 1);
      } /* End of 'RotateX' funciton */

      /* Rotation around Y axis transformation matrix setup function.
       * ARGUMENTS:
       *   - rotation angle in degrees:
       *       Type AngleInDegree;
       * RETURNS:
       *   (matr4<Type>) result matrix.
       */
      inline static matr4<Type> RotateY( Type AngleInDegree )
      {
        Type sinx, cosx;
        sincos(AngleInDegree, sinx, cosx);
        return matr4<Type>( cosx, 0, sinx, 0,
                               0, 1,    0, 0,
                           -sinx, 0, cosx, 0,
                               0, 0,    0, 1);
      } /* End of 'RotateY' funciton */

      /* Rotation around Z axis transformation matrix setup function.
       * ARGUMENTS:
       *   - rotation angle in degrees:
       *       Type AngleInDegree;
       * RETURNS:
       *   (matr4<Type>) result matrix.
       */
      inline static matr4<Type> RotateZ( Type AngleInDegree )
      {
        Type sinx, cosx;
        sincos(AngleInDegree, sinx, cosx);
        return matr4<Type>(cosx, -sinx, 0, 0,
                           sinx,  cosx, 0, 0,
                              0,     0, 1, 0,
                              0,     0, 0, 1);
      } /* End of 'RotateZ' funciton */

      /* Decompose matrix to its elements.
       * ARGUMENTS:
       * RETURNS:
       *   (vec3<Type>) Rotation per each axis in degrees.
       */
      vec3<Type> GetEulerAngles()
      {
        vec3<Type> Ans;
        Type sy = sqrt(A[0][0] * A[0][0] + A[1][0] * A[1][0]);
        bool is_singular = sy < 0.00005;

        if (!is_singular)
        {
          Ans.X = atan2(A[2][1], A[2][2]);
          Ans.Y = atan2(-A[2][0], sy);
          Ans.Z = atan2(A[1][0], A[0][0]);
        }
        else
        {
          Ans.X = atan2(-A[1][2], A[1][1]);
          Ans.Y = atan2(-A[2][0], sy);
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
       *   (vec3<Type>) Translation vector.
       *   (vec3<Type>) Rotation per each axis in degrees.
       *   (vec3<Type>) Scale per each axis.
       */
      void Decompose(vec3<Type>& Translation, vec3<Type>& RotationPerAxis, vec3<Type> &Scale) const
      {
        Type sx = vec3<Type>(A[0][0], A[0][1], A[0][2]).Lenght() ;
        Type sy = vec3<Type>(A[1][0], A[1][1], A[1][2]).Lenght() ;
        Type sz = vec3<Type>(A[2][0], A[2][1], A[2][2]).Lenght() ;

        // if determine is negative, we need to invert one scale
        Type det = Determ();
        if (det < 0) {
          sx = -sx;
        }

        // Translation vector
        Translation.X = A[3][0];
        Translation.Y = A[3][1];
        Translation.Z = A[3][2];

        // Scale rotation part
        Type invSX = 1.0 / sx;
        Type invSY = 1.0 / sy;
        Type invSZ = 1.0 / sz;

        matr4<Type> tmpMatr = *this;

        tmpMatr.A[0][0] *= invSX;
        tmpMatr.A[0][1] *= invSX;
        tmpMatr.A[0][2] *= invSX;

        tmpMatr.A[1][0] *= invSY;
        tmpMatr.A[1][1] *= invSY;
        tmpMatr.A[1][2] *= invSY;

        tmpMatr.A[2][0] *= invSZ;
        tmpMatr.A[2][1] *= invSZ;
        tmpMatr.A[2][2] *= invSZ;
        
        RotationPerAxis = tmpMatr.GetEulerAngles();

        Scale.X = sx;
        Scale.Y = sy;
        Scale.Z = sz;
      }

      /* Decompose matrix to its elements.
       * ARGUMENTS:
       * RETURNS:
       *   (vec3<Type>) Translation vector.
       *   (vec3<Type>) Rotation per each axis in degrees.
       *   (vec3<Type>) Scale per each axis.
       */
      void Decompose(vec3<Type>& Translation, vec4<Type>& Quaternion, vec3<Type>& Scale) const
      {
        Type sx = vec3<Type>(A[0][0], A[0][1], A[0][2]).Lenght();
        Type sy = vec3<Type>(A[1][0], A[1][1], A[1][2]).Lenght();
        Type sz = vec3<Type>(A[2][0], A[2][1], A[2][2]).Lenght();

        // if determine is negative, we need to invert one scale
        Type det = Determ();
        if (det < 0) {
          sx = -sx;
        }

        // Translation vector
        Translation.X = A[3][0];
        Translation.Y = A[3][1];
        Translation.Z = A[3][2];

        // Scale rotation part
        Type invSX = 1.0 / sx;
        Type invSY = 1.0 / sy;
        Type invSZ = 1.0 / sz;

        matr4<Type> tmpMatr = *this;

        tmpMatr.A[0][0] *= invSX;
        tmpMatr.A[0][1] *= invSX;
        tmpMatr.A[0][2] *= invSX;

        tmpMatr.A[1][0] *= invSY;
        tmpMatr.A[1][1] *= invSY;
        tmpMatr.A[1][2] *= invSY;

        tmpMatr.A[2][0] *= invSZ;
        tmpMatr.A[2][1] *= invSZ;
        tmpMatr.A[2][2] *= invSZ;

        float tr = tmpMatr.A[0][0] + tmpMatr.A[1][1] + tmpMatr.A[2][2];

        if (tr > 0) {
          float S = sqrt(tr + 1.0) * 2; // S=4*qw 
          Quaternion.W = 0.25 * S;
          Quaternion.X = (tmpMatr.A[1][2] - tmpMatr.A[2][1]) / S;
          Quaternion.Y = (tmpMatr.A[2][0] - tmpMatr.A[0][2]) / S;
          Quaternion.Z = (tmpMatr.A[0][1] - tmpMatr.A[1][0]) / S;
        }
        else if ((tmpMatr.A[0][0] > tmpMatr.A[1][1]) & (tmpMatr.A[0][0] > tmpMatr.A[2][2])) {
          float S = sqrt(1.0 + tmpMatr.A[0][0] - tmpMatr.A[1][1] - tmpMatr.A[2][2]) * 2; // S=4*qx 
          Quaternion.W = (tmpMatr.A[1][2] - tmpMatr.A[2][1]) / S;
          Quaternion.X = 0.25 * S;
          Quaternion.Y = (tmpMatr.A[1][0] + tmpMatr.A[0][1]) / S;
          Quaternion.Z = (tmpMatr.A[2][0] + tmpMatr.A[0][2]) / S;
        }
        else if (tmpMatr.A[1][1] > tmpMatr.A[2][2]) {
          float S = sqrt(1.0 + tmpMatr.A[1][1] - tmpMatr.A[0][0] - tmpMatr.A[2][2]) * 2; // S=4*qy
          Quaternion.W = (tmpMatr.A[2][0] - tmpMatr.A[0][2]) / S;
          Quaternion.X = (tmpMatr.A[1][0] + tmpMatr.A[0][1]) / S;
          Quaternion.Y = 0.25 * S;
          Quaternion.Z = (tmpMatr.A[2][1] + tmpMatr.A[1][2]) / S;
        }
        else {
          float S = sqrt(1.0 + tmpMatr.A[2][2] - tmpMatr.A[0][0] - tmpMatr.A[1][1]) * 2; // S=4*qz
          Quaternion.W = (tmpMatr.A[0][1] - tmpMatr.A[1][0]) / S;
          Quaternion.X = (tmpMatr.A[2][0] + tmpMatr.A[0][2]) / S;
          Quaternion.Y = (tmpMatr.A[2][1] + tmpMatr.A[1][2]) / S;
          Quaternion.Z = 0.25 * S;
        }

        Scale.X = sx;
        Scale.Y = sy;
        Scale.Z = sz;
      }

      /* Translation matrix setup function.
       * ARGUMENTS:
       *   - translation vector:
       *       const vec3 &Vec;
       * RETURNS:
       *   (matr4<Type>) result translation matrix.
       */
      inline static matr4<Type> Translate( const vec3<Type> &Vec )
      {
        return matr4<Type>(    1,     0,     0, 0,
                               0,     1,     0, 0,
                               0,     0,     1, 0,
                           Vec.X, Vec.Y, Vec.Z, 1);
      } /* End of 'Translate' function */

      /* Scale transformation matrix initializing function .
       * ARGUMENTS:
       *   - scale vector:
       *       const vec3 &Vec;
       * RETURNS:
       *   (vec) scale transformation initialized matrix structure.
       */
      inline static matr4<Type> Scale( const vec3<Type> &Vec )
      {
        return matr4<Type>(Vec.X,     0,     0, 0,
                               0, Vec.Y,     0, 0,
                               0,     0, Vec.Z, 0,
                               0,     0,     0, 1);
      } /* End of 'Scale' funciton */

      /* Rotation around arbitraty axis function.
       * ARGUMENTS:
       *   - angle in degrees:
       *       Type AngleInDegree;
       *   - axis:
       *       const vec3 &Axis;
       * RETURNS:
       *   (MATR) result rotation matrix.
       */
      inline static matr4<Type> Rotate( Type AngleInDegree, const vec3<Type> &Axis )
      { 
        Type sinx, cosx;
        matr4<Type> m = matr4<Type>::Identity();
        sincos(AngleInDegree, sinx, cosx);
        m[0][0] = cosx + Axis.X * Axis.X * (1 - cosx);
        m[1][0] = Axis.X * Axis.Y * (1 - cosx) - Axis.Z * sinx;
        m[2][0] = Axis.Z * Axis.X * (1 - cosx) + Axis.Y * sinx;
        m[0][1] = Axis.Y * Axis.X * (1 - cosx) + Axis.Z * sinx;
        m[1][1] = cosx + Axis.Y * Axis.Y * (1 - cosx);
        m[2][1] = Axis.Z * Axis.Y * (1 - cosx) - Axis.X * sinx;
        m[0][2] = Axis.Z * Axis.X * (1 - cosx) - Axis.Y * sinx;
        m[1][2] = Axis.Z * Axis.Y * (1 - cosx) + Axis.X * sinx;
        m[2][2] = cosx + Axis.Z * Axis.Z * (1 - cosx);
        return m;
      } /* End of 'Rotate' function */

      /* View transformation matrix initializing function .
       * ARGUMENTS:
       *   - view transformation parametrs:
       *      vec Loc, At, Up;
       * RETURNS:
       *   (matr4<Type>) view transformation initialized matrix structure.
       */
      inline static matr4<Type> View( vec3<Type> Loc, vec3<Type> At, vec3<Type> Up )
      {
        vec3<Type>
          D = (At - Loc).Normalized(),
          R = (D cross Up).Normalized(),
          U = (R cross D).Normalized();
    
        return matr4<Type>(         R.X,         U.X,       D.X, 0,
                                    R.Y,         U.Y,       D.Y, 0,
                                    R.Z,         U.Z,       D.Z, 0,
                             -Loc dot R,   -Loc dot U, -Loc dot D, 1);
      } /* End of 'View' function */

      /* Orthographic transformation matrix initializing function .
       * ARGUMENTS:
       *   - orthographic transformation matrix parametrs:
       *       Type 
       *         Left, Right, 
       *         Bottom, Top,
       *         Near,  Far;
       * RETURNS:
       *   (matr4<Type>) orthographic transformation initialized matrix structure.
       */
      inline static matr4<Type> Ortho( Type Left, Type Right, Type Bottom, Type Top, Type Near, Type Far )
      {
        return matr4<Type>(              2 / (Right - Left),                                0,                            0, 0,
                                                          0,               2 / (Top - Bottom),                            0, 0,
                                                          0,                                0,            1 / (Far - Near), 0,
                           -(Right + Left) / (Right - Left), -(Top + Bottom) / (Top - Bottom), 0, 1);
      } /* End of 'Ortho' function */

      /* Frustum transformation matrix initializing function.
       * ARGUMENTS:
       *   - frustum transformation matrix parametrs:
       *       Type 
       *         Left, Right,
       *         Bottom, Top,
       *         Near,  Far;
       * RETURNS:
       *   (matr4<Type>) frustum transformation initialized matrix structure.
       */
      inline static matr4<Type> Frustum( Type Left, Type Right, Type Bottom, Type Top, Type Near, Type Far )
      {
        return matr4<Type>((Type)2.0 * Near / (Right - Left),                           (Type)0,                   (Type)0, (Type)0,
                                                     (Type)0, (Type)2.0 * Near / (Top - Bottom),                   (Type)0, (Type)0,
                                                     (Type)0,   (Top + Bottom) / (Top - Bottom),        Far / (Far - Near), (Type)1,
                                                     (Type)0,                           (Type)0, Far * Near / (Near - Far), (Type)0);
      } /* End of 'Frustum' function */

      /* Get matrix line funciton.
       * ARGUMENTS:
       *   - index of line:
       *       UINT Index;
       * RETURNS:
       *   (Type *) matrix line.
       */
      inline Type * operator[]( UINT Index )
      {
        return A[Index];
      } /* End of 'operator[]' funciton */

      /* Get constant matrix line funciton.
       * ARGUMENTS:
       *   - index of line:
       *       UINT Index;
       * RETURNS:
       *   (Type *) matrix line.
       */
      inline const Type * operator[]( UINT Index ) const
      {
        return A[Index];
      } /* End of 'operator[]' funciton */

      /* Matrix multiplication funciton.
       * ARGUMENTS:
       *   - matrix to be multiplated to:
       *       const matr4<Type> &Matr;
       * RETURNS:
       *   (matr4<Type>) result matrix.
       */
      inline matr4<Type> operator*( const matr4<Type> &Matr ) const
      {
        matr4<Type> m(0.0f);
        for (UINT i = 0; i < 4; i++)
          for (UINT j = 0; j < 4; j++)
            for (UINT k = 0; k < 4; k++)
              m.A[i][j] += A[i][k] * Matr.A[k][j];
        return m;
      } /* End of 'operator*' fucntion */

      /* Matrix multiplication funciton.
       * ARGUMENTS:
       *   - matrix to be multiplated to:
       *       const matr4<Type> &Matr;
       * RETURNS:
       *   (matr4<Type> &) link to this matrix.
       */
      inline matr4<Type> & operator*=( const matr4<Type> &Matr )
      {
        matr4<Type> m = *this;
        memset(A, 0, sizeof(FLOAT) * 16);
        for (UINT i = 0; i < 4; i++)
          for (UINT j = 0; j < 4; j++)
            for (UINT k = 0; k < 4; k++)
              A[i][j] += m.A[k][j] * Matr.A[i][k];
        return *this;
      } /* End of 'operator*=' fucntion */

      /* Matrix determinant funcion
       * ARGUMENTS: None.
       * RETURNS:
       *   (Type) determinant.
       */
      inline Type Determ( VOID ) const
      {
        return
          A[0][0] * Determ3x3(A[1][1], A[1][2], A[1][3],
                              A[2][1], A[2][2], A[2][3],
                              A[3][1], A[3][2], A[3][3]) -
          A[0][1] * Determ3x3(A[1][0], A[1][2], A[1][3],
                              A[2][0], A[2][2], A[2][3],
                              A[3][0], A[3][2], A[3][3]) +
          A[0][2] * Determ3x3(A[1][0], A[1][1], A[1][3],
                              A[2][0], A[2][1], A[2][3],
                              A[3][0], A[3][1], A[3][3]) -
          A[0][3] * Determ3x3(A[1][0], A[1][1], A[1][2],
                              A[2][0], A[2][1], A[2][2],
                              A[3][0], A[3][1], A[3][2]);
      } /* End of 'Determ' function */

      /* Matrix invertion funcion
       * ARGUMENTS: None.
       * RETURNS:
       *   (matr4<Type>) inversed matrix.
       */
      inline matr4<Type> Inversed( VOID ) const
      {
        return matr4<Type>(*this).Inverse();
      } /* End of 'Inversed' function */

      /* Current matrix invertion funcion
       * ARGUMENTS: None.
       * RETURNS:
       *   (matr4<Type> &) link to this matrix.
       */
      inline matr4<Type> & Inverse( VOID )
      {
        matr4<Type> r;
        Type det = Determ();
        INT p[4][3] =
        {
          {1, 2, 3},
          {0, 2, 3},
          {0, 1, 3},
          {0, 1, 2}
        };

        if (det == 0)
          return *this;

        for (int i = 0; i < 4; i++)
          for (int j = 0; j < 4; j++)  
            r.A[j][i] =
              (1 - (i + j) % 2 * 2) *
              Determ3x3(A[p[i][0]][p[j][0]], A[p[i][0]][p[j][1]], A[p[i][0]][p[j][2]],
                        A[p[i][1]][p[j][0]], A[p[i][1]][p[j][1]], A[p[i][1]][p[j][2]],
                        A[p[i][2]][p[j][0]], A[p[i][2]][p[j][1]], A[p[i][2]][p[j][2]]) / det;
        *this = r;
        return *this;
      } /* End of 'Inverse' function */

      /* Current matrix transpose function.
       * ARGUMENTS: None.
       * RETURNS:
       *   (matr4<Type> &) link to this matrix.
       */
      inline matr4<Type> & Transpose( VOID )
      {
        matr4<Type> M = *this;
    
        for (UINT i = 0; i < 4; i++)
          for (UINT j = 0; j < 4; j++)
            A[i][j] = M.A[j][i];
        return *this;
      } /* End of 'Transpose' function */

      /* Matrix transpose function.
       * ARGUMENTS: None.
       * RETURNS:
       *   (matr4<Type>) transposed matrix.
       */
      inline matr4<Type> Transposed( VOID ) const
      {
        return matr4<Type>(*this).Transpose();
      } /* End of 'Transposed' function */


      /* op*= */
     auto operator*=( const Type &S )
     {
       for (UINT i = 0; i < 4; i++)
         for (UINT j = 0; j < 4; j++)
         {
           this->A[i][j] *= S;
         }

       return *this;
     }

      /* Matrix equality funciton.
       * ARGUMENTS:
       *   - matrix to be comparised with:
       *       const matr4<Type> &Matr;
       * RETURNS:
       *   (BOOL) who of them is Bazhen??.
       */
      inline BOOL operator==( const matr4<Type> &Matr ) const
      {
        return memcmp(Matr.A, A, sizeof(A)) == 0;
      } /* End of 'operator==' function */
    }; /* End of 'matr4' class */
} /* End of 'mth' namespace */

#endif /* __mth_matr_h_ */
