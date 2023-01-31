#ifndef __mth_cam_h_
#define __mth_cam_h_

/* Math namespace */
namespace mth
{
  /* Matrices helper */
  template<class Type>
  struct MATRICES
  {
    /* Camera matrices */
    matr4<Type>
      MatrView,              /* View matrix */
      MatrProj;              /* Project matrix */
    matr4<Type> MatrVP;    /* View, proj matrixes multiplication */
  }; /* End of 'MATRICES' struct */

/* Camera class */
  template<class Type>
  class cam3 : public MATRICES<Type>
  {
  protected:
    /* Camera project plane dparameters */
    Type
      W, H,               /* Width, height of camera project plane*/
      ProjDist, FarClip,  /* Project distance, farclip distance */
      Size;               /* Project plane shortest side size */

    /* Camera space position and orientation */
    vec3<Type>
      Loc, Dir,           /* Location of camera, direction of camera */
      Up, Right,          /* Directions at up, at right */
      LookAt;             /* Camera point look at */

    /* Camera view frame parameters */
    int
      FrameW, FrameH;     /* Frame size */

    mutable bool ChangeFlag; /* Is changed flag */
  public:
    /* Coordinate systems enum */
    enum COORINATE_SYSTEM
    {
      WORLD,
      CAMERA,
      SCREEN
    };  /* End of 'COORINATE_SYSTEM' enum */

  private:
    COORINATE_SYSTEM CurrentCS; /* Current coordinate system */

  public:
    /* Camera constructor.
     * ARGUMENTS:
     *   - parametrs for initialize view matrix:
     *       const vec3<Type> &Loc, &LookAt, &Up;
     *   - camera project plane minimal side size:
     *       Type Size;
     *   - near and far clip distance of view frustum values:
     *       Type ProjDist, FarClip;
     *   - window size in pixels:
     *       int FrameW, FrameH;
     */
    cam3(const vec3<Type>& Loc, const vec3<Type>& LookAt, const vec3<Type>& Up, Type Size, Type ProjDist, Type FarClip, int FrameW, int FrameH) : Loc(Loc), LookAt(LookAt), FrameW(FrameW), FrameH(FrameH), ChangeFlag(true), CurrentCS(COORINATE_SYSTEM::WORLD)
    {
      /* Setup view */
      SetView(Loc, LookAt, Up);
      SetProj(Size, ProjDist, FarClip);
    } /* End of 'cam3' funciton */

    /* Camera project matrix setup function.
     * ARGUMENTS:
     *   - camera project plane minimal side size:
     *       Type Size;
     *   - near and far clip distance of view frustum values:
     *       Type ProjDist, FarClip;
     * RETURNS:
     *   (cam3 &) link to this cam.
     */
    cam3& SetProj(Type Size, Type ProjDist, Type FarClip)
    {
      this->Size = Size;
      this->ProjDist = ProjDist;
      this->FarClip = FarClip;
      if (FrameW > FrameH)
        W = Size * FrameW / FrameH, H = Size;
      else
        W = Size, H = Size * FrameH / FrameW;

      switch (CurrentCS)
      {
      case COORINATE_SYSTEM::WORLD:
        MATRICES<Type>::MatrProj = matr4<Type>::Frustum(-W / 2, W / 2, -H / 2, H / 2, ProjDist, FarClip);
        break;
      case COORINATE_SYSTEM::SCREEN:
        MATRICES<Type>::MatrProj = matr4<Type>::Ortho((Type)0, (Type)(FrameW - 1), (Type)(FrameH - 1), (Type)0, (Type)(-ProjDist), (Type)(FarClip));
        break;
      case COORINATE_SYSTEM::CAMERA:
        MATRICES<Type>::MatrProj = matr4<Type>::Identity();
        break;
      }
      ChangeFlag = true;
      return *this;
    } /* End of 'SetProj' function */

    /* Camera resize function.
     * ARGUMENTS: None.
     *   - frame size:
     *       int FrameW, FrameH;
     * RETURNS:
     *   (cam3 &) link to this camera instanse.
     */
    cam3& Resize(int FrameW, int FrameH)
    {
      this->FrameH = FrameH;
      this->FrameW = FrameW;
      SetProj(this->Size, this->ProjDist, this->FarClip);
      return *this;
    } /* End of 'Resize' function */

    /* Camera view matrix setup function.
     * ARGUMENTS:
     *   - position, point look at, approximately direction on up:
     *       vec3<Type> Loc, LookAt, Up;
     * RETURNS:
     *   (cam3 &) link to this cam.
     */
    cam3& SetView(vec3<Type> Loc, vec3<Type> LookAt, vec3<Type> Up)
    {
      switch (CurrentCS)
      {
      case COORINATE_SYSTEM::WORLD:
        MATRICES<Type>::MatrView = matr4<Type>::View(Loc, LookAt, Up);
        break;
      case COORINATE_SYSTEM::SCREEN:
      case COORINATE_SYSTEM::CAMERA:
        MATRICES<Type>::MatrView = matr4<Type>::Identity();
        break;
      default:
        break;
      }
      this->Loc = Loc;
      this->LookAt = LookAt;
      this->Up = Up;
      this->Dir = (LookAt - Loc).Normalized();
      this->Right = (Dir cross Up).Normalized();
      ChangeFlag = true;
      return *this;
    } /* End of 'SetView' function */

    /* Camera view proj matrix get function.
     * ARGUMENTS: None.
     * RETURNS:
     *   (matr4<Type> &) link to VP martix.
     */
    const matr4<Type>& GetVP(void)
    {
      if (ChangeFlag)
        MATRICES<Type>::MatrVP = MATRICES<Type>::MatrView * MATRICES<Type>::MatrProj, ChangeFlag = false;
      return MATRICES<Type>::MatrVP;
    } /* End of 'GetVP' function */

    /* Camera position get function.
     * ARGUMENTS: None.
     * RETURNS:
     *   (vec3<Type> &) contant link to position vector.
     */
    const vec3<Type>& GetPos(void) const
    {
      return Loc;
    } /* End of 'GetPos' function */

    /* Camera up vector get function.
     * ARGUMENTS: None.
     * RETURNS:
     *   (vec3<Type> &) contant link to up vector.
     */
    const vec3<Type>& GetUp(void) const
    {
      return Up;
    } /* End of 'GetUp' function */

    /* Camera direction vector get function.
     * ARGUMENTS: None.
     * RETURNS:
     *   (vec3<Type> &) contant link to direction vector.
     */
    const vec3<Type>& GetDir(void) const
    {
      return Dir;
    } /* End of 'GetDir' funciton */

    /* Camera right vector get function.
     * ARGUMENTS: None.
     * RETURNS:
     *   (vec3<Type> &) contant link to right vector.
     */
    const vec3<Type>& GetRight(void) const
    {
      return Right;
    } /* End of 'GetDir' funciton */

    /* Camera lookat vector get function.
     * ARGUMENTS: None.
     * RETURNS:
     *   (vec3<Type> &) contant link to lookat vector.
     */
    const vec3<Type>& GetLookAt(void) const
    {
      return LookAt;
    } /* End of 'GetDir' funciton */

    /* Camera near clip get function.
     * ARGUMENTS: None.
     * RETURNS:
     *  (Type) near clip.
     */
    Type GetNear(void) const
    {
      return ProjDist;
    } /* End of 'GetNear' function */

    /* Camera far plane get function.
     * ARGUMENTS: None.
     * RETURNS:
     *  (Type) far plane.
     */
    Type GetFar(void) const
    {
      return FarClip;
    } /* End of 'GetFar' function */

    /* Rotate around X axis funciton
     * ARGUMENTS:
     *   - angle in degree to rotate:
     *       Type AngleInDegree;
     * RETURNS:
     *   (cam3 &) link to this cam.
     */
    cam3& RotateX(Type AngleInDegree)
    {
      matr4<Type> rot = matr4<Type>::RotateX(AngleInDegree);
      SetView(Loc * rot, LookAt * rot, Up * rot);
      return *this;
    } /* End of 'RotateX' function */

    /* Rotate around Y axis funciton
     * ARGUMENTS:
     *   - angle in degree to rotate:
     *       Type AngleInDegree;
     * RETURNS:
     *   (cam3 &) link to this cam.
     */
    cam3& RotateY(Type AngleInDegree)
    {
      matr4<Type> rot = matr4<Type>::RotateY(AngleInDegree);
      SetView(Loc * rot, LookAt * rot, Up * rot);
      return *this;
    } /* End of 'RotateY' function */

    /* Rotate around Z axis funciton
     * ARGUMENTS:
     *   - angle in degree to rotate:
     *       Type AngleInDegree;
     * RETURNS:
     *   (cam3 &) link to this cam.
     */
    cam3& RotateZ(Type AngleInDegree)
    {
      matr4<Type> rot = matr4<Type>::RotateZ(AngleInDegree);
      SetView(Loc * rot, LookAt * rot, Up * rot);
      return *this;
    } /* End of 'RotateZ' function */

    /* Rotate around Loc Up funciton
     * ARGUMENTS:
     *   - angle in degree to rotate:
     *       Type AngleInDegree;
     * RETURNS:
     *   (cam3 &) link to this cam.
     */
    cam3& RotateAroundLocY(Type AngleInDegree)
    {
      matr4<Type> rot = matr4<Type>::Rotate(AngleInDegree, Up);
      SetView(Loc, (LookAt - Loc) * rot + Loc, Up);
      return *this;
    } /* End of 'RotateAroundLocUp' function */

    /* Rotate around Loc Right funciton
     * ARGUMENTS:
     *   - angle in degree to rotate:
     *       Type AngleInDegree;
     * RETURNS:
     *   (cam3 &) link to this cam.
     */
    cam3& RotateAroundLocRight(Type AngleInDegree)
    {
      matr4<Type> rot = matr4<Type>::Rotate(AngleInDegree, Right);
      SetView(Loc, (LookAt - Loc) * rot + Loc, Up);
      return *this;
    } /* End of 'RotateAroundLocRight' function */

    /* Rotate around LookAt Up axis funciton
     * ARGUMENTS:
     *   - angle in degree to rotate:
     *       Type AngleInDegree;
     * RETURNS:
     *   (cam3 &) link to this cam.
     */
    cam3& RotateAroundAtY(Type AngleInDegree)
    {
      matr4<Type> rot = matr4<Type>::Rotate(AngleInDegree, Up);
      vec3<Type> Up1 = vec3<Type>(MATRICES<Type>::MatrView[0][1], MATRICES<Type>::MatrView[1][1], MATRICES<Type>::MatrView[2][1]);
      SetView((Loc - LookAt) * rot + LookAt, LookAt, Up);
      return *this;
    } /* End of 'RotateAroundAtUp' function */

    /* Rotate around LookAt Right axis funciton
     * ARGUMENTS:
     *   - angle in degree to rotate:
     *       Type AngleInDegree;
     * RETURNS:
     *   (cam3 &) link to this cam.
     */
    cam3& RotateAroundAtRight(Type AngleInDegree)
    {
      matr4<Type> rot = matr4<Type>::Rotate(AngleInDegree, Right);
      SetView((Loc - LookAt) * rot + LookAt, LookAt, Up);
      return *this;
    } /* End of 'RotateAroundAtRight' function */

    /* Move camera in world cs funciton
     * ARGUMENTS:
     *   - translation vector:
     *       const vec3<Type> &Translation;
     * RETURNS:
     *   (cam3 &) link to this cam.
     */
    cam3& Translate(const vec3<Type>& Translation)
    {
      SetView(Loc + Translation, LookAt + Translation, Up);
      return *this;
    } /* End of 'Translate' function */

    /* Move camera in world cs funciton
     * ARGUMENTS:
     *   - translation vector:
     *       const vec3<Type> &Translation;
     * RETURNS:
     *   (cam3 &) link to this cam.
     */
    cam3& SetPos(const vec3<Type>& Position)
    {
      SetView(Position, Position + Dir, Up);
      return *this;
    } /* End of 'SetPos' function */

    /* Move camera in world cs funciton
     * ARGUMENTS:
     *   - translation vector:
     *       const vec3<Type> &Translation;
     * RETURNS:
     *   (cam3 &) link to this cam.
     */
    cam3& SetDir(const vec3<Type>& Direction)
    {
      SetView(Loc, Loc + Direction, Up);
      return *this;
    } /* End of 'SetDir' function */

    /* Move camera in camera cs funciton
     * ARGUMENTS:
     *   - translation vector:
     *       const vec3<Type> &Translation;
     * RETURNS:
     *   (cam3 &) link to this cam.
     */
    cam3& TranslateCamera(const vec3<Type>& Translation)
    {
      vec3<Type> Delta = Right * Translation.X + Up * Translation.Y + Dir * Translation.Z;

      SetView(Loc + Delta, LookAt + Delta, Up);
      return *this;
    } /* End of 'TranslateCamera' function */

    /* Move camera location in camera cs funciton
     * ARGUMENTS:
     *   - translation vector:
     *       const vec3<Type> &Translation;
     * RETURNS:
     *   (cam3 &) link to this cam.
     */
    cam3& TranslateLocCamera(const vec3<Type>& Translation)
    {
      vec3<Type> Delta = Right * Translation.X + Up * Translation.Y + Dir * Translation.Z;

      SetView(Loc + Delta, LookAt, Up);
      return *this;
    } /* End of 'TranslateCamera' function */

    /* Set camera zoom function
     * ARGUMENTS:
     *   - zoom value:
     *       const Type &Value;
     * RETURNS:
     *   (cam3 &) link to this cam.
     */
    inline cam3& SetZoom(const Type& Value)
    {
      assert(Value > 0);
      vec3<Type> Delta = Dir.Normalized() * Value;

      SetView(LookAt - Delta, LookAt, Up);
      return *this;
    } /* End of 'SetZoom' function */

    /* Camera zoom function
     * ARGUMENTS:
     *   - zoom value:
     *       const Type &Value;
     * RETURNS:
     *   (cam3 &) link to this cam.
     */
    inline cam3& Zoom(const Type& Value)
    {
      Type v = (Loc - LookAt).Lenght() + Value;
      if (v < 0.001f)
        v = 0.001f;
      vec3<Type> Delta = Dir.Normalized() * v;

      SetView(LookAt - Delta, LookAt, Up);
      return *this;
    } /* End of 'Zoom' function */

    /* Camera coordinate system change funciton
     * ARGUMENTS:
     *   - coordinate system:
     *       COORINATE_SYSTEM CS;
     * RETURNS:
     *   (cam3 &) link to this cam.
     */
    cam3& SetCoordinateSystem(COORINATE_SYSTEM CS)
    {
      if (CS == CurrentCS)
        return *this;
      ChangeFlag = true;
      CurrentCS = CS;
      SetProj(Size, ProjDist, FarClip);
      SetView(Loc, LookAt, Up);
      return *this;
    } /* End of 'SetCoordinateSystem' function */

    /* Camera coordinate system get funciton
     * ARGUMENTS: None.
     * RETURNS:
     *   (camera::COORINATE_SYSTEM) current coordinate system.
     */
    COORINATE_SYSTEM GetCoordinateSystem(void)
    {
      return CurrentCS;
    } /* End of 'GetCoordinateSystem' function */
  }; /* End of 'cam3' class */
} /* End of 'mth' namespace */

#endif /* __mth_cam_h_*/