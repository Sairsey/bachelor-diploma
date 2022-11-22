#ifndef __mth_vec_h_
#define __mth_vec_h_

#include <assert.h>
#include <iostream>
#include <sstream>


/* Math namespace */
namespace mth
{
  /* 2d vector class */
  template<class Type>
    class vec2
    {
    public:
      /* Vector components */
      union
      {
        Type X;
        Type U;
      };
    
      union
      {
        Type Y;
        Type V;
      };
    
      /* Default constructor
       * ARGUMENTS:
       *   - value for all conmonents:
       *       Type A = 0;
       */
      inline vec2<Type>( Type A = 0 ) : X(A), Y(A)
      {
      } /* End of 'vec2<Type>' function */
    
      /* 2 component constructor
       * ARGUMENTS:
       *   - values for conmonents:
       *       Type X, Y;
       */
      inline vec2<Type>( Type X, Type Y ) : X(X), Y(Y)
      {
      } /* End of 'vec2<Type>' function */
    
      /* Vector lenght calculation funciton
       * ARGUMENTS: None.
       * RETURNS:
       *   (Type) vector lenght.
       */
      inline Type Lenght( VOID ) const
      {
        return sqrt(X * X + Y * Y);
      } /* End of 'Lenght' funciton */
    
      /* Vector normalization funciton
       * ARGUMENTS: None.
       * RETURNS:
       *   (vec2<Type> &) link to this vector.
       */
      inline vec2<Type> & Normalize( VOID )
      {
        Type Len = Lenght();
        if (Len != 0 && Len != 1)
        {
          X /= Len;
          Y /= Len;
        }
        return *this;
      } /* End of 'Normalize' function */
    
      /* Vector normalization funciton
       * ARGUMENTS: None.
       * RETURNS:
       *   (vec2<Type>) normalized vector.
       */
      inline vec2<Type> Normalized( VOID ) const
      {
        return vec2<Type>(*this).Normalize();
      } /* End of 'mth::vec2<Type>::Normalized' function */
    
      /* Vector compare function
       * ARGUMENTS:
       *   - vector to be compared:
       *       const vec2<Type> &Vec;
       * RETURNS:
       *   (BOOL) compare result
       */
      inline BOOL operator==( const vec2<Type> &Vec ) const
      {
        return COM_ABS((*this - Vec).Lenght()) < 0.00005;
      } /* End of 'operator==' function */
    
      /* Vector compare function
       * ARGUMENTS:
       *   - vector to be compared:
       *       const vec2<Type> &Vec;
       * RETURNS:
       *   (BOOL) compare result
       */
      inline BOOL operator!=( const vec2<Type> &Vec ) const
      {
        return !(*this == Vec);
      } /* End of 'operator!=' function */
    
      /* Vector slogenie function 
       * ARGUMENTS:
       *   - vector to be added:
       *       const vec2<Type> &Vec;
       * RETURNS:
       *   (vec2<Type> &) link to this vector.
       */
      inline vec2<Type> & operator+=( const vec2<Type> &Vec )
      {
        X += Vec.X;
        Y += Vec.Y;
        return *this;
      } /* End of 'operator+=' function */
    
      /* Vector slogenie function 
       * ARGUMENTS:
       *   - vector to be added:
       *       const vec2<Type> &Vec;
       * RETURNS:
       *   (vec2<Type>) result vector.
       */
      inline vec2<Type> operator+( const vec2<Type> &Vec ) const
      {
        return vec2<Type>(Vec.X + X, Vec.Y + Y);
      } /* End of 'operator+' function */
    
      /* Vector substraction function
       * ARGUMENTS:
       *   - vector to be substracted:
       *       const vec2<Type> &Vec;
       * RETURNS:
       *   (vec2<Type> &) link to this vector.
       */
      inline vec2<Type> & operator-=( const vec2<Type> &Vec )
      {
        X -= Vec.X;
        Y -= Vec.Y;
        return *this;
      } /* End of 'operator-=' function */
    
      /* Vector substraction function
       * ARGUMENTS:
       *   - vector to be substracted:
       *       const vec2<Type> &Vec;
       * RETURNS:
       *   (vec2<Type>) result vector.
       */
      inline vec2<Type> operator-( const vec2<Type> &Vec ) const
      {
        return vec2<Type>(X - Vec.X, Y - Vec.Y);
      } /* End of 'mth::vec2<Type>::operator-' function */
    
      /* Vector multiplication function
       * ARGUMENTS:
       *   - vector to be multiplicated:
       *       const vec2<Type> &Vec;
       * RETURNS:
       *   (vec2<Type> &) link to this vector.
       */
      inline vec2<Type> & operator*=( const vec2<Type> &Vec )
      {
        X *= Vec.X;
        Y *= Vec.Y;
        return *this;
      } /* End of 'operator*=' function */
    
      /* Vector multiplication function
       * ARGUMENTS:
       *   - vector to be multiplicated:
       *       const vec2<Type> &Vec;
       * RETURNS:
       *   (vec2<Type>) result vector.
       */
      inline vec2<Type> operator*( const vec2<Type> &Vec ) const
      {
        return vec2<Type>(Vec.X * X, Vec.Y * Y);
      } /* End of 'operator*' function */
    
      /* Vector division function
       * ARGUMENTS:
       *   - vector to be multiplicated:
       *       const vec2<Type> &Vec;
       * RETURNS:
       *   (vec2<Type> &) link to this vector.
       */
      inline vec2<Type> & operator/=( const vec2<Type> &Vec )
      {
        assert(Vec.X != 0 && Vec.Y != 0);
        X /= Vec.X;
        Y /= Vec.Y;
        return *this;
      } /* End of 'operator/=' function */
    
      /* Vector division function
       * ARGUMENTS:
       *   - vector to be divided:
       *       const vec2<Type> &Vec;
       * RETURNS:
       *   (vec2<Type>) result vector.
       */
      inline vec2<Type> operator/( const vec2<Type> &Vec ) const
      {
        assert(Vec.X != 0 && Vec.Y != 0);
        return vec2<Type>(X / Vec.X, Y / Vec.Y);
      } /* End of 'operator/' function */
    
      /* Vector slogenie number function 
       * ARGUMENTS:
       *   - number to be added:
       *       Type Num;
       * RETURNS:
       *   (vec2<Type> &) link to this vector.
       */
      inline vec2<Type> & operator+=( Type Num )
      {
        X += Num;
        Y += Num;
        return *this;
      } /* End of 'mth::vec2<Type>::operator+=' function */
    
      /* Vector slogenie number function 
       * ARGUMENTS:
       *   - number to be added:
       *       Type Num;
       * RETURNS:
       *   (vec2<Type>) result vector.
       */
      inline vec2<Type> operator+( Type Num ) const
      {
        return vec2<Type>(X + Num, Y + Num);
      } /* End of 'operator+' function */
    
      /* Vector substraction number function
       * ARGUMENTS:
       *   - number to be substracted:
       *       Type Num;
       * RETURNS:
       *   (vec2<Type> &) link to this vector.
       */
      inline vec2<Type> & operator-=( Type Num )
      {
        X -= Num;
        Y -= Num;
        return *this;
      } /* End of 'operator-=' function */
    
      /* Vector substraction number function
       * ARGUMENTS:
       *   - number to be substracted:
       *       Type Num;
       * RETURNS:
       *   (vec2<Type>) result vector.
       */
      inline vec2<Type> operator-( Type Num ) const
      {
        return vec2<Type>(X - Num, Y - Num);
      } /* End of 'operator-' function */
    
      /* Vector multiplication number function
       * ARGUMENTS:
       *   - number to be multiplicated:
       *       Type Num;
       * RETURNS:
       *   (vec2<Type> &) link to this vector.
       */
      inline vec2<Type> & operator*=( Type Num )
      {
        X *= Num;
        Y *= Num;
        return *this;
      } /* End of 'operator*=' function */
    
      /* Vector multiplication number function
       * ARGUMENTS:
       *   - number to be multiplicated:
       *       Type Num;
       * RETURNS:
       *   (vec2<Type>) result vector.
       */
      inline vec2<Type> operator*( Type Num ) const
      {
        return vec2<Type>(X * Num, Y * Num);
      } /* End of 'operator*' function */
    
      /* Vector division number function
       * ARGUMENTS:
       *   - number to be divided:
       *       Type Num;
       * RETURNS:
       *   (vec2<Type> &) link to this vector.
       */
      inline vec2<Type> & operator/=( Type Num )
      {
        assert(Num != 0);
        X /= Num;
        Y /= Num;
        return *this;
      } /* End of 'operator/=' function */
    
      /* Vector division number function
       * ARGUMENTS:
       *   - number to be divided:
       *       Type Num;
       * RETURNS:
       *   (vec2<Type>) result vector.
       */
      inline vec2<Type> operator/( Type Num ) const
      {
        assert(Num != 0);
        return vec2<Type>(X / Num, Y / Num);
      } /* End of 'operator/' function */
    
      /* Vector negative function
       * ARGUMENTS: None.
       * RETURNS:
       *   (vec2<Type>) result vector.
       */
      inline vec2<Type> operator-( VOID ) const
      {
        return vec2<Type>(-X, -Y);
      }; /* End of 'operator-' function */
    
      /* Get link to vector component funciton
       * ARGUMENTS:
       *   - number of component:
       *       UINT Index;
       * RETURNS:
       *   (Type &) link to component.
       */
      inline Type & operator[]( UINT Index )
      {
        assert(Index >= 0 && Index <= 1);
        switch (Index)
        {
        case 0:
          return X;
        case 1:
          return Y;
        }
        return X;
      } /* End of 'operator[]' funciton */
    
      /* Dot product function.
       * ARGUMENTS:
       *   - vector to be dot to:
       *       const vec2<Type> &Vec;
       * RETURNS:
       *   (Type) dot product of vectors.
       */
      inline Type operator&( const vec2<Type> &Vec ) const
      {
        return (X * Vec.X + Y * Vec.Y);
      } /* End of 'operator&' function */
  }; /* End of 'vec2' class */

  
  /* 4d vector class */
  template<class Type>
    class vec4
    {
    public:
      /* Vector components */
      union
      {
        Type X;
        Type R;
      };
      union
      {
        Type Y;
        Type G;
      };
      union
      {
        Type Z;
        Type B;
      };
      union
      {
        Type W;
        Type A;
      };

    public:
      /* Default constructor
       * ARGUMENTS:
       *   - value for all conmonents:
       *       Type A = 1.0;
       */
      inline vec4<Type>( Type A = 1.0 ) : X(A), Y(A), Z(A), W(A)
      {
      } /* End of 'vec4<Type>' funciton */

      /* 3 component constructor
       * ARGUMENTS:
       *   - values for conmonents:
       *       Type X, Y, Z, W;
       */
      inline vec4<Type>( Type X, Type Y, Type Z, Type W ) : X(X), Y(Y), Z(Z), W(W)
      {
      } /* End of 'vec4<Type>' function */

      /* Vector lenght calculation funciton
       * ARGUMENTS: None.
       * RETURNS:
       *   (Type) vector lenght.
       */
      inline Type Lenght( VOID ) const
      {
        return sqrt(X * X + Y * Y + Z * Z + W * W);
      } /* End of 'Lenght' funciton */

      /* Vector normalization funciton
       * ARGUMENTS: None.
       * RETURNS:
       *   (vec4<Type> &) link to this vector.
       */
      inline vec4<Type> & Normalize( VOID )
      {
        Type Len = Lenght();
        if (Len != 0 && Len != 1)
        {
          X /= Len;
          Y /= Len;
          Z /= Len;
          W /= Len;
        }
        return *this;
      } /* End of 'Normalize' function */

      /* Vector normalization funciton
       * ARGUMENTS: None.
       * RETURNS:
       *   (vec4<Type>) normalized vector.
       */
      inline vec4<Type> Normalized( VOID ) const
      {
        return vec4<Type>(*this).Normalize();
      } /* End of 'Normalized' function */

      /* Vector compare function
       * ARGUMENTS:
       *   - vector to be compared:
       *       const vec4<Type> &Vec;
       * RETURNS:
       *   (BOOL) compare result
       */
      inline BOOL operator==( const vec4<Type> &Vec ) const
      {
        return COM_ABS((*this - Vec).Lenght()) < 0.00005;
      } /* End of 'operator==' function */

      /* Vector compare function
       * ARGUMENTS:
       *   - vector to be compared:
       *       const vec4<Type> &Vec;
       * RETURNS:
       *   (BOOL) compare result
       */
      inline BOOL operator!=( const vec4<Type> &Vec ) const
      {
        return !(*this == Vec);
      } /* End of 'operator!=' function */

      /* Vector slogenie function 
       * ARGUMENTS:
       *   - vector to be added:
       *       const vec4<Type> &Vec;
       * RETURNS:
       *   (vec4<Type> &) link to this vector.
       */
      inline vec4<Type> & operator+=( const vec4<Type> &Vec )
      {
        X += Vec.X;
        Y += Vec.Y;
        Z += Vec.Z;
        W += Vec.W;
        return *this;
      } /* End of 'operator+=' function */

      /* Vector slogenie function 
       * ARGUMENTS:
       *   - vector to be added:
       *       const vec4<Type> &Vec;
       * RETURNS:
       *   (vec4<Type>) result vector.
       */
      inline vec4<Type> operator+( const vec4<Type> &Vec ) const
      {
        return vec4<Type>(Vec.X + X, Vec.Y + Y, Vec.Z + Z, Vec.W + W);
      } /* End of 'operator+' function */

      /* Vector substraction function
       * ARGUMENTS:
       *   - vector to be substracted:
       *       const vec4<Type> &Vec;
       * RETURNS:
       *   (vec4<Type> &) link to this vector.
       */
      inline vec4<Type> & operator-=( const vec4<Type> &Vec )
      {
        X -= Vec.X;
        Y -= Vec.Y;
        Z -= Vec.Z;
        W -= Vec.W;
        return *this;
      } /* End of 'operator-=' function */

      /* Vector substraction function
       * ARGUMENTS:
       *   - vector to be substracted:
       *       const vec4<Type> &Vec;
       * RETURNS:
       *   (vec4<Type>) result vector.
       */
      inline vec4<Type> operator-( const vec4<Type> &Vec ) const
      {
        return vec4<Type>(X - Vec.X, Y - Vec.Y, Z - Vec.Z, W - Vec.W);
      } /* End of 'operator-' function */

      /* Vector multiplication function
       * ARGUMENTS:
       *   - vector to be multiplicated:
       *       const vec4<Type> &Vec;
       * RETURNS:
       *   (vec4<Type> &) link to this vector.
       */
      inline vec4<Type> & operator*=( const vec4<Type> &Vec )
      {
        X *= Vec.X;
        Y *= Vec.Y;
        Z *= Vec.Z;
        W *= Vec.W;
        return *this;
      } /* End of 'operator*=' function */

      /* Vector multiplication function
       * ARGUMENTS:
       *   - vector to be multiplicated:
       *       const vec4<Type> &Vec;
       * RETURNS:
       *   (vec4<Type>) result vector.
       */
      inline vec4<Type> operator*( const vec4<Type> &Vec ) const
      {
        return vec4<Type>(Vec.X * X, Vec.Y * Y, Vec.Z * Z, Vec.W * W);
      } /* End of 'operator*' function */

      /* Vector division function
       * ARGUMENTS:
       *   - vector to be multiplicated:
       *       const vec4<Type> &Vec;
       * RETURNS:
       *   (vec4<Type> &) link to this vector.
       */
      inline vec4<Type> & operator/=( const vec4<Type> &Vec )
      {
        assert(Vec.X != 0 && Vec.Y != 0 && Vec.Z != 0 && Vec.W != 0);
        X /= Vec.X;
        Y /= Vec.Y;
        Z /= Vec.Z;
        W /= Vec.W;
        return *this;
      } /* End of 'operator/=' function */

      /* Vector division function
       * ARGUMENTS:
       *   - vector to be divided:
       *       const vec4<Type> &Vec;
       * RETURNS:
       *   (vec4<Type>) result vector.
       */
      inline vec4<Type> operator/( const vec4<Type> &Vec ) const
      {
        assert(Vec.X != 0 && Vec.Y != 0 && Vec.Z != 0 && Vec.W != 0);
        return vec4<Type>(X / Vec.X, Y / Vec.Y, Z / Vec.Z, W / Vec.W);
      } /* End of 'operator/' function */

      /* Vector slogenie number function 
       * ARGUMENTS:
       *   - number to be added:
       *       Type Num;
       * RETURNS:
       *   (vec4<Type> &) link to this vector.
       */
      inline vec4<Type> & operator+=( Type Num )
      {
        X += Num;
        Y += Num;
        Z += Num;
        W += Num;
        return *this;
      } /* End of 'operator+=' function */

      /* Vector slogenie number function 
       * ARGUMENTS:
       *   - number to be added:
       *       Type Num;
       * RETURNS:
       *   (vec4<Type>) result vector.
       */
      inline vec4<Type> operator+( Type Num ) const
      {
        return vec4<Type>(X + Num, Y + Num, Z + Num , W + Num);
      } /* End of 'operator+' function */

      /* Vector substraction number function
       * ARGUMENTS:
       *   - number to be substracted:
       *       Type Num;
       * RETURNS:
       *   (vec4<Type> &) link to this vector.
       */
      inline vec4<Type> & operator-=( Type Num )
      {
        X -= Num;
        Y -= Num;
        Z -= Num;
        W -= Num;
        return *this;
      } /* End of 'operator-=' function */

      /* Vector substraction number function
       * ARGUMENTS:
       *   - number to be substracted:
       *       Type Num;
       * RETURNS:
       *   (vec4<Type>) result vector.
       */
      inline vec4<Type> operator-( Type Num ) const
      {
        return vec4<Type>(X - Num, Y - Num, Z - Num, W - Num);
      } /* End of 'operator-' function */

      /* Vector multiplication number function
       * ARGUMENTS:
       *   - number to be multiplicated:
       *       Type Num;
       * RETURNS:
       *   (vec4<Type> &) link to this vector.
       */
      inline vec4<Type> & operator*=( Type Num )
      {
        X *= Num;
        Y *= Num;
        Z *= Num;
        W *= Num;
        return *this;
      } /* End of 'operator*=' function */

      /* Vector multiplication number function
       * ARGUMENTS:
       *   - number to be multiplicated:
       *       Type Num;
       * RETURNS:
       *   (vec4<Type>) result vector.
       */
      inline vec4<Type> operator*( Type Num ) const
      {
        return vec4<Type>(X * Num, Y * Num, Z * Num, W * Num);
      } /* End of 'operator*' function */

      /* Vector division number function
       * ARGUMENTS:
       *   - number to be divided:
       *       Type Num;
       * RETURNS:
       *   (vec4<Type> &) link to this vector.
       */
      inline vec4<Type> & operator/=( Type Num )
      {
        assert(Num != 0);
        X /= Num;
        Y /= Num;
        Z /= Num;
        W /= Num;
        return *this;
      } /* End of 'operator/=' function */

      /* Vector division number function
       * ARGUMENTS:
       *   - number to be divided:
       *       Type Num;
       * RETURNS:
       *   (vec4<Type>) result vector.
       */
      inline vec4<Type> operator/( Type Num ) const
      {
        assert(Num != 0);
        return vec4<Type>(X / Num, Y / Num, Z / Num, W / Num);
      } /* End of 'operator/' function */

      /* Vector negative function
       * ARGUMENTS: None.
       * RETURNS:
       *   (vec4<Type>) result vector.
       */
      inline vec4<Type> operator-( VOID ) const
      {
        return vec4<Type>(-X, -Y, -Z, -W);
      } /* End of 'operator-' function */

      /* Get link to vector component funciton
       * ARGUMENTS:
       *   - number of component:
       *       UINT Index;
       * RETURNS:
       *   (Type &) link to component.
       */
      inline Type & operator[]( UINT Index )
      {
        assert(Index >= 0 && Index <= 3);
        switch (Index)
        {
        case 0:
          return X;
        case 1:
          return Y;
        case 2:
          return Z;
        case 3:
          return W;
        }
        return X;
      } /* End of 'operator[]' funciton */

      /* Dot product function.
       * ARGUMENTS:
       *   - vector to be dot to:
       *       const vec4<Type> &Vec;
       * RETURNS:
       *   (Type) dot product of vectors.
       */
      inline Type operator&( const vec4<Type> &Vec ) const
      {
        return (X * Vec.X + Y * Vec.Y + Z * Vec.Z + W * Vec.W);
      } /* End of 'operator&' function */

      /* Cross product of three 4 components vectors function.
       * ARGUMENTS:
       *   - vectors:
       *       const vec3<Type> &U,
       *       const vec3<Type> &V,
       *       const vec3<Type> &W;
       * RETURNS:
       *   (vec3<Type>) vector cross product of vectors.
       */
      inline static vec4<Type> Cross(const vec4<Type> &U, const vec4<Type> &V, const vec4<Type> &W)
      {
        DBL A, B, C, D, E, F;       // Intermediate Values

        // Calculate intermediate values.
        A = (V.X * W.Y) - (V.Y * W.X);
        B = (V.X * W.Z) - (V.Z * W.X);
        C = (V.X * W.W) - (V.W * W.X);
        D = (V.Y * W.Z) - (V.Z * W.Y);
        E = (V.Y * W.W) - (V.W * W.Y);
        F = (V.Z * W.W) - (V.W * W.Z);

        // Calculate the result-vector components.
        return vec4<Type>((U.Y * F) - (U.Z * E) + (U.W * D),
                         -(U.X * F) + (U.Z * C) - (U.W * B),
                         (U.X * E) - (U.Y * C) + (U.W * A),
                         -(U.X * D) + (U.Y * B) - (U.Z * A));
      } /* End of 'Cross' function */

      // Spherical linear interpolation between unit quaternions q1 and q2 with interpolation parameter t.
      vec4<Type> slerp(vec4<Type> q2, float t)
      {
        float w1, x1, y1, z1, w2, x2, y2, z2, w3, x3, y3, z3;
        vec4<Type> q2New;
        float theta, mult1, mult2;

        w1 = W; x1 = X; y1 = Y; z1 = Z;
        w2 = q2.W; x2 = q2.X; y2 = q2.Y; z2 = q2.Z;

        // Reverse the sign of q2 if q1.q2 < 0.
        if (w1 * w2 + x1 * x2 + y1 * y2 + z1 * z2 < 0)
        {
          w2 = -w2; x2 = -x2; y2 = -y2; z2 = -z2;
        }

        theta = acos(w1 * w2 + x1 * x2 + y1 * y2 + z1 * z2);

        if (theta > 0.000001)
        {
          mult1 = sin((1 - t) * theta) / sin(theta);
          mult2 = sin(t * theta) / sin(theta);
        }

        // To avoid division by 0 and by very small numbers the approximation of sin(angle)
        // by angle is used when theta is small (0.000001 is chosen arbitrarily).
        else
        {
          mult1 = 1 - t;
          mult2 = t;
        }

        w3 = mult1 * w1 + mult2 * w2;
        x3 = mult1 * x1 + mult2 * x2;
        y3 = mult1 * y1 + mult2 * y2;
        z3 = mult1 * z1 + mult2 * z2;

        return vec4<Type>(x3, y3, z3, w3);
      }
    }; /* End of 'vec4' class */

  /* 3d vector class */
  template<class Type>
    class vec3
    {
    public:
      /* Vector components */
      union
      {
        Type X;
        Type R;
      };
   
      union
      {
        Type Y;
        Type G;
      };
   
      union
      {
        Type Z;
        Type B;
      };
   
    public:


      /* Casting operator */
      template  <typename Type1>
        operator vec3<Type1>( VOID )
        {
          return vec3<Type1>(static_cast<Type1>(X), static_cast<Type1>(Y), static_cast<Type1>(Z));
        } /* End of 'operator vec<Type1>' function */

      /* Default constructor
       * ARGUMENTS:
       *   - value for all conmonents:
       *       Type A = 0;
       */
      inline vec3<Type>( Type A = 0 ) : X(A), Y(A), Z(A)
      {
      } /* End of 'vec3<Type>' funciton */

      /* Constructor to be prim worck with 4d
       * ARGUMENTS:
       *   - 4d vector:
       *       const vec4<Type> &Vec;
       */
      inline vec3<Type>( const vec4<Type> &Vec ) : X(Vec.X), Y(Vec.Y), Z(Vec.Z)
      {
        if (Vec.W != 0)
        {
          X /= Vec.W;
          Y /= Vec.W;
          Z /= Vec.W;
        }
      } /* End of 'vec3<Type>' funciton */

      /* Constructor to be prim worck with 2d
       * ARGUMENTS:
       *   - 2d vector:
       *       const vec2<Type> &Vec;
       */
      inline vec3<Type>( const vec2<Type> &Vec ) : X(Vec.X), Y(Vec.Y)
      {
      } /* End of 'vec3<Type>' funciton */

      /* 3 component constructor
       * ARGUMENTS:
       *   - values for conmonents:
       *       Type X, Y, Z;
       */
      inline vec3<Type>( Type X, Type Y, Type Z ) : X(X), Y(Y), Z(Z)
      {
      } /* End of 'vec3<Type>' function */
   
      /* Vector lenght calculation funciton
       * ARGUMENTS: None.
       * RETURNS:
       *   (Type) vector lenght.
       */
      inline Type Lenght( VOID ) const
      {
        return sqrt(X * X + Y * Y + Z * Z);
      } /* End of 'Lenght' funciton */
   
      /* Vector normalization funciton
       * ARGUMENTS: None.
       * RETURNS:
       *   (vec3<Type> &) link to this vector.
       */
      inline vec3<Type> & Normalize( VOID )
      {
        Type Len = Lenght();
   
        if (Len != 0 && Len != 1)
        {
          X /= Len;
          Y /= Len;
          Z /= Len;
        }
        return *this;
      } /* End of 'Normalize' function */

      /* Vector cosine interpolation whith this vector funciton
       * ARGUMENTS:
       *   - Second vector to interpolation:
       *       const vec3<Type> &X;
       *   - Mu coef:
       *       const Type Mu;
       * RETURNS:
       *   (vec3<Type>) new interpolated vector.
       */
      inline vec3<Type> CosineInterpolate( const vec3<Type> &X, const Type Mu )
      {
        double mu;

        mu = (1 - cos(Mu * MTH_PI)) / 2;
        return (*this * (1 - mu) + X *mu);
      } /* End of 'CosineInterpolate' function */
   
      /* Vector normalization funciton
       * ARGUMENTS: None.
       * RETURNS:
       *   (vec3<Type>) normalized vector.
       */
      inline vec3<Type> Normalized( VOID ) const
      {
        return vec3<Type>(*this).Normalize();
      } /* End of 'Normalized' function */
   
      /* Vector compare function
       * ARGUMENTS:
       *   - vector to be compared:
       *       const vec3<Type> &Vec;
       * RETURNS:
       *   (BOOL) compare result
       */
      inline BOOL operator==( const vec3<Type> &Vec ) const
      {
        return std::abs((*this - Vec).Lenght()) < 0.00005f;
      } /* End of 'operator==' function */
   
      /* Vector compare function
       * ARGUMENTS:
       *   - vector to be compared:
       *       const vec3<Type> &Vec;
       * RETURNS:
       *   (BOOL) compare result
       */
      inline BOOL operator!=( const vec3<Type> &Vec ) const
      {
        return !(*this == Vec);
      } /* End of 'operator!=' function */
   
      /* Vector slogenie function 
       * ARGUMENTS:
       *   - vector to be added:
       *       const vec3<Type> &Vec;
       * RETURNS:
       *   (vec3<Type> &) link to this vector.
       */
      inline vec3<Type> & operator+=( const vec3<Type> &Vec )
      {
        X += Vec.X;
        Y += Vec.Y;
        Z += Vec.Z;
        return *this;
      } /* End of 'operator+=' function */
   
      /* Vector slogenie function 
       * ARGUMENTS:
       *   - vector to be added:
       *       const vec3<Type> &Vec;
       * RETURNS:
       *   (vec3<Type>) result vector.
       */
      inline vec3<Type> operator+( const vec3<Type> &Vec ) const
      {
        return vec3<Type>(Vec.X + X, Vec.Y + Y, Vec.Z + Z);
      } /* End of 'operator+' function */
   
      /* Vector slogenie function 
       * ARGUMENTS:
       *   - vector to be added:
       *       const vec2<Type> &Vec;
       * RETURNS:
       *   (vec3<Type> &) link to this vector.
       */
      inline vec3<Type> & operator+=( const vec2<Type> &Vec )
      {
        X += Vec.X;
        Y += Vec.Y;
        return *this;
      } /* End of 'operator+=' function */
   
      /* Vector slogenie function 
       * ARGUMENTS:
       *   - vector to be added:
       *       const vec2<Type> &Vec;
       * RETURNS:
       *   (vec3<Type>) result vector.
       */
      inline vec3<Type> operator+( const vec2<Type> &Vec ) const
      {
        return vec3<Type>(Vec.X + X, Vec.Y + Y, Z);
      } /* End of 'operator+' function */
   
      /* Vector substraction function
       * ARGUMENTS:
       *   - vector to be substracted:
       *       const vec3<Type> &Vec;
       * RETURNS:
       *   (vec3<Type> &) link to this vector.
       */
      inline vec3<Type> & operator-=( const vec3<Type> &Vec )
      {
        X -= Vec.X;
        Y -= Vec.Y;
        Z -= Vec.Z;
        return *this;
      } /* End of 'operator-=' function */
   
      /* Vector substraction function
       * ARGUMENTS:
       *   - vector to be substracted:
       *       const vec3<Type> &Vec;
       * RETURNS:
       *   (vec3<Type>) result vector.
       */
      inline vec3<Type> operator-( const vec3<Type> &Vec ) const
      {
        return vec3<Type>(X - Vec.X, Y - Vec.Y, Z - Vec.Z);
      } /* End of 'operator-' function */
   
      /* Vector multiplication function
       * ARGUMENTS:
       *   - vector to be multiplicated:
       *       const vec3<Type> &Vec;
       * RETURNS:
       *   (vec3<Type> &) link to this vector.
       */
      inline vec3<Type> & operator*=( const vec3<Type> &Vec )
      {
        X *= Vec.X;
        Y *= Vec.Y;
        Z *= Vec.Z;
        return *this;
      } /* End of 'operator*=' function */
   
      /* Vector multiplication function
       * ARGUMENTS:
       *   - vector to be multiplicated:
       *       const vec3<Type> &Vec;
       * RETURNS:
       *   (vec3<Type>) result vector.
       */
      inline vec3<Type> operator*( const vec3<Type> &Vec ) const
      {
        return vec3<Type>(Vec.X * X, Vec.Y * Y, Vec.Z * Z);
      } /* End of 'operator*' function */
   
      /* Vector division function
       * ARGUMENTS:
       *   - vector to be multiplicated:
       *       const vec3<Type> &Vec;
       * RETURNS:
       *   (vec3<Type> &) link to this vector.
       */
      inline vec3<Type> & operator/=( const vec3<Type> &Vec )
      {
        assert(Vec.X != 0 && Vec.Y != 0 && Vec.Z != 0);
        X /= Vec.X;
        Y /= Vec.Y;
        Z /= Vec.Z;
        return *this;
      } /* End of 'operator/=' function */
   
      /* Vector division function
       * ARGUMENTS:
       *   - vector to be divided:
       *       const vec3<Type> &Vec;
       * RETURNS:
       *   (vec3<Type>) result vector.
       */
      inline vec3<Type> operator/( const vec3<Type> &Vec ) const
      {
        assert(Vec.X != 0 && Vec.Y != 0 && Vec.Z != 0);
        return vec3<Type>(X / Vec.X, Y / Vec.Y, Z / Vec.Z);
      } /* End of 'operator/' function */
   
      /* Vector slogenie number function 
       * ARGUMENTS:
       *   - number to be added:
       *       Type Num;
       * RETURNS:
       *   (vec3<Type> &) link to this vector.
       */
      inline vec3<Type> & operator+=( Type Num )
      {
        X += Num;
        Y += Num;
        Z += Num;
        return *this;
      } /* End of 'operator+=' function */
   
      /* Vector slogenie number function 
       * ARGUMENTS:
       *   - number to be added:
       *       Type Num;
       * RETURNS:
       *   (vec3<Type>) result vector.
       */
      inline vec3<Type> operator+( Type Num ) const
      {
        return vec3<Type>(X + Num, Y + Num, Z + Num);
      } /* End of 'operator+' function */
   
      /* Vector substraction number function
       * ARGUMENTS:
       *   - number to be substracted:
       *       Type Num;
       * RETURNS:
       *   (vec3<Type> &) link to this vector.
       */
      inline vec3<Type> & operator-=( Type Num )
      {
        X -= Num;
        Y -= Num;
        Z -= Num;
        return *this;
      } /* End of 'operator-=' function */
   
      /* Vector substraction number function
       * ARGUMENTS:
       *   - number to be substracted:
       *       Type Num;
       * RETURNS:
       *   (vec3<Type>) result vector.
       */
      inline vec3<Type> operator-( Type Num ) const
      {
        return vec3<Type>(X - Num, Y - Num, Z - Num);
      } /* End of 'operator-' function */
   
      /* Vector multiplication number function
       * ARGUMENTS:
       *   - number to be multiplicated:
       *       Type Num;
       * RETURNS:
       *   (vec3<Type> &) link to this vector.
       */
      inline vec3<Type> & operator*=( Type Num )
      {
        X *= Num;
        Y *= Num;
        Z *= Num;
        return *this;
      } /* End of 'operator*=' function */
   
      /* Vector multiplication number function
       * ARGUMENTS:
       *   - number to be multiplicated:
       *       Type Num;
       * RETURNS:
       *   (vec3<Type>) result vector.
       */
      inline vec3<Type> operator*( Type Num ) const
      {
        return vec3<Type>(X * Num, Y * Num, Z * Num);
      } /* End of 'operator*' function */
   
      /* Vector division number function
       * ARGUMENTS:
       *   - number to be divided:
       *       Type Num;
       * RETURNS:
       *   (vec3<Type> &) link to this vector.
       */
      inline vec3<Type> & operator/=( Type Num )
      {
        assert(Num != 0);
        X /= Num;
        Y /= Num;
        Z /= Num;
        return *this;
      } /* End of 'operator/=' function */
   
      /* Vector division number function
       * ARGUMENTS:
       *   - number to be divided:
       *       Type Num;
       * RETURNS:
       *   (vec3<Type>) result vector.
       */
      inline vec3<Type> operator/( Type Num ) const
      {
        assert(Num != 0);
        return vec3<Type>(X / Num, Y / Num, Z / Num);
      } /* End of 'operator/' function */
   
      /* Vector negative function
       * ARGUMENTS: None.
       * RETURNS:
       *   (vec3<Type>) result vector.
       */
      inline vec3<Type> operator-( VOID ) const
      {
        return vec3<Type>(-X, -Y, -Z);
      } /* End of 'operator-' function */
   
      /* Get link to vector component funciton
       * ARGUMENTS:
       *   - number of component:
       *       UINT Index;
       * RETURNS:
       *   (Type &) link to component.
       */
      inline Type & operator[]( UINT Index )
      {
        assert(Index >= 0 && Index <= 2);
        switch (Index)
        {
        case 0:
          return X;
        case 1:
          return Y;
        case 2:
          return Z;
        }
        return X;
      } /* End of 'operator[]' funciton */
   
      /* Dot product function.
       * ARGUMENTS:
       *   - vector to be dot to:
       *       const vec3<Type> &Vec;
       * RETURNS:
       *   (Type) dot product of vectors.
       */
      inline Type operator&( const vec3<Type> &Vec ) const
      {
        return (X * Vec.X + Y * Vec.Y + Z * Vec.Z);
      } /* End of 'operator&' function */
   
      /* Cross product of two 3 components vectors function.
       * ARGUMENTS:
       *   - vector:
       *       const vec3<Type> &Vec;
       * RETURNS:
       *   (vec3<Type>) vector cross product of vectors.
       */
      inline vec3<Type> operator%( const vec3<Type> &Vec ) const
      {
        return vec3<Type>(Y * Vec.Z - Z * Vec.Y, Z * Vec.X - X * Vec.Z, X * Vec.Y - Y * Vec.X);
      } /* End of 'operator%' function */
    }; /* End of 'vec3' class */
} /* End of 'mth' namespace */
#endif /* __mth_vec_h__ */
