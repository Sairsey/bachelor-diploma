#ifndef __mth_quat_h_
#define __mth_quat_h_

/* Math namespace */
namespace mth
{
  template < class Type>
    class quat
    {
    public:
      Type Q1, Q2, Q3, Q4;   // vector components

      /* Default constructor.
       * ARGUMENTS: None.
       */
      inline quat( VOID ) : Q1(1), Q2(0), Q3(0), Q4(0)
      {
      } /* End of 'quat' function */

      /* 2 component constructor
       * ARGUMENTS:
       *   - Vector of quat:
       *       const vec3<Type> &Vector;
       *   - Q1 of quat
       *       const Type &Q1;
       */
      inline quat( const Type &Q1, const vec3<Type> &Vector) : Q2(Q2), Q3(Q3), Q4(Q4), Q1(Q1)
      {
      } /* End of 'quat' function */

      /* 4 component constructor
       * ARGUMENTS:
       *   - Quaternion values:
       *       Type W, X, Y, Z;
       */
      inline quat( const Type &W, const Type &X, const Type &Y, const Type &Z ) : Q2(X), Q3(Y), Q4(Z), Q1(W)
      {
      } /* End of 'quat' function */

      /* Quaternion multiplication function
       * ARGUMENTS:
       *   - quaternion to be multiplicated:
       *       const quat<Type> &Q;
       * RETURNS:
       *   (quat<Type>) result quaternion.
       */
      inline quat<Type> operator*( const quat<Type> &Q ) const
      {
        quat out;

        out.Q1 = (Q.Q1 * Q1) - (Q.Q2 * Q2) - (Q.Q3 * Q3) - (Q.Q4 * Q4);
        out.Q2 = (Q.Q1 * Q2) + (Q.Q2 * Q1) + (Q.Q3 * Q4) - (Q.Q4 * Q3);
        out.Q3 = (Q.Q1 * Q3) + (Q.Q3 * Q1) + (Q.Q4 * Q2) - (Q.Q2 * Q4);
        out.Q4 = (Q.Q1 * Q4) + (Q.Q4 * Q1) + (Q.Q2 * Q3) - (Q.Q3 * Q2);
        return out;
      } /* End of 'operator*' function */

      /* Quaternion sum operator. 
       * ARGUMENTS:
       *   - another quaternion:
       *       const Type1 &Quat;
       * RETURNS:
       *   (auto) result quaternion.
       */
      template <typename Type1>
        auto operator+( const quat<Type1> Quat ) const
        {
          return quat{
            Quat.Q1 + Q1,
            Quat.Q2 + Q2,
            Quat.Q3 + Q3,
            Quat.Q4 + Q4
          };
        } /* End of 'operator+' function */

      /* Quaternion substraction operator. 
       * ARGUMENTS:
       *   - another quaternion:
       *       const Type1 &Quat;
       * RETURNS:
       *   (auto) result quaternion.
       */
      template <typename Type1>
        auto operator-( const quat<Type1> Quat ) const
        {
          return quat{
            Quat.Q1 - Q1,
            Quat.Q2 - Q2,
            Quat.Q3 - Q3,
            Quat.Q4 - Q4
          };
        } /* End of 'operator-' function */

      /* Quaternion multiplication by value operator. 
       * ARGUMENTS:
       *   - value:
       *       const Type1 &Value;
       * RETURNS:
       *   (auto) result quaternion.
       */
      template <typename Type1>
        auto operator*( const Type1 Value ) const
        {
          return quat{
            Q1 * Value,
            Q2 * Value,
            Q3 * Value,
            Q4 * Value
          };
        } /* End of 'operator*' function */



      /* Quaternion normalization function
       * ARGUMENTS: None.
       * RETURNS:
       *   (quat &) link to this class instance.
       */
      inline quat & Normalize( VOID )
      {
        Type L = Q1 * Q1 + Q2 * Q2 + Q3 * Q3 + Q4 * Q4;

        if (L == 0 || L == 1)
          return *this;
        L = std::sqrt(L);
        Q1 /= L;
        Q2 /= L;
        Q3 /= L;
        Q4 /= L;

        return *this;
      } /* End of 'Normalize' function */

      /* Normalized quaternion function
       * ARGUMENTS: None.
       * RETURNS:
       *   (quat) link to this class instance.
       */
      inline quat Normalized( VOID )
      {
        return quat(*this).Normalize();
      }

    }; /* End of 'quat' class */
} /* End of 'mth' namespace */

#endif /* __mth_quat_h_ */
