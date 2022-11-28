#include "p_header.h"

void gdr::gdr_physics_object::ToggleRotation(void)
{
  physx::PxRigidDynamic *BodyReal = Body->is<physx::PxRigidDynamic>();
  IsLockedRotation = !IsLockedRotation;
  physx::PxRigidDynamicLockFlags flags;
  if (IsLockedRotation)
    flags = flags | physx::PxRigidDynamicLockFlag::eLOCK_ANGULAR_X | physx::PxRigidDynamicLockFlag::eLOCK_ANGULAR_Y | physx::PxRigidDynamicLockFlag::eLOCK_ANGULAR_Z;
  if (IsLockedTranslation)
    flags = flags | physx::PxRigidDynamicLockFlag::eLOCK_LINEAR_X | physx::PxRigidDynamicLockFlag::eLOCK_LINEAR_Y | physx::PxRigidDynamicLockFlag::eLOCK_LINEAR_Z;
  BodyReal->setRigidDynamicLockFlags(flags);
}
void gdr::gdr_physics_object::ToggleTranslation(void)
{
  physx::PxRigidDynamic *BodyReal = Body->is<physx::PxRigidDynamic>();
  IsLockedTranslation = !IsLockedTranslation;
  physx::PxRigidDynamicLockFlags flags;
  if (IsLockedRotation)
    flags = flags | physx::PxRigidDynamicLockFlag::eLOCK_ANGULAR_X | physx::PxRigidDynamicLockFlag::eLOCK_ANGULAR_Y | physx::PxRigidDynamicLockFlag::eLOCK_ANGULAR_Z;
  if (IsLockedTranslation)
    flags = flags | physx::PxRigidDynamicLockFlag::eLOCK_LINEAR_X | physx::PxRigidDynamicLockFlag::eLOCK_LINEAR_Y | physx::PxRigidDynamicLockFlag::eLOCK_LINEAR_Z;
  BodyReal->setRigidDynamicLockFlags(flags);
}
mth::matr4f gdr::gdr_physics_object::GetTransform(void)
{
  physx::PxTransform Trans = Body->getGlobalPose();
  return mth::matr4f::FromQuaternionAndPosition({Trans.q.x, Trans.q.y, Trans.q.z, Trans.q.w}, {Trans.p[0], Trans.p[1], Trans.p[2]});
}
void gdr::gdr_physics_object::ChangeDensity(float Density)
{
  if (IsStatic)
    return;
  physx::PxRigidBody *BodyReal = Body->is<physx::PxRigidBody>();
  physx::PxRigidBodyExt::updateMassAndInertia(*BodyReal, Density);
}
void gdr::gdr_physics_object::ChangeRotation(mth::matr4f Rot)
{
  physx::PxRigidBody *BodyReal = Body->is<physx::PxRigidBody>();
  physx::PxVec3 Pos = BodyReal->getGlobalPose().p;
  float arr[9] = {
      Rot[0][0], Rot[0][1], Rot[0][2],
      Rot[1][0], Rot[1][1], Rot[1][2],
      Rot[2][0], Rot[2][1], Rot[2][2] };
  physx::PxMat33 RotM = physx::PxMat33(arr);
  BodyReal->setGlobalPose(physx::PxTransform(Pos, physx::PxQuat(RotM)));
}
void gdr::gdr_physics_object::AddVelocity(float3 Vel)
{
  if (IsStatic)
    return;
  physx::PxRigidBody *BodyReal = Body->is<physx::PxRigidBody>();
  
  physx::PxVec3 OldVelocity = BodyReal->getLinearVelocity();
  OldVelocity[0] += Vel[0];
  OldVelocity[1] += Vel[1];
  OldVelocity[2] += Vel[2];
  BodyReal->setLinearVelocity(OldVelocity);
  return;
}

void gdr::gdr_physics_object::SetVelocity(float3 Vel)
{
  if (IsStatic)
    return;
  physx::PxRigidBody* BodyReal = Body->is<physx::PxRigidBody>();

  physx::PxVec3 OldVelocity = BodyReal->getLinearVelocity();
  OldVelocity[0] = Vel[0];
  OldVelocity[1] = Vel[1];
  OldVelocity[2] = Vel[2];
  BodyReal->setLinearVelocity(OldVelocity);
  return;
}

float3 gdr::gdr_physics_object::GetVelocity(void)
{
  if (IsStatic)
    return {0, 0, 0};
  physx::PxRigidBody* BodyReal = Body->is<physx::PxRigidBody>();
  physx::PxVec3 OldVelocity = BodyReal->getLinearVelocity();
  return {OldVelocity[0], OldVelocity[1], OldVelocity[2]};
}

void gdr::gdr_physics_object::SetPos(float3 Pos)
{
  if (IsStatic)
    return;
  physx::PxRigidBody *BodyReal = Body->is<physx::PxRigidBody>();
  physx::PxQuat Rot = BodyReal->getGlobalPose().q;
  BodyReal->setGlobalPose(physx::PxTransform({ Pos[0], Pos[1], Pos[2] }, Rot));
  return;
}
void gdr::gdr_physics_object::Stop(void)
{
  if (IsStatic)
    return;

  physx::PxRigidBody *BodyReal = Body->is<physx::PxRigidBody>();
  BodyReal->setLinearVelocity(physx::PxVec3(0));
  BodyReal->setAngularVelocity(physx::PxVec3(0));
  return;
}

void gdr::gdr_physics_object::SetCollideCallback(std::function<void(gdr_physics_object* Me, gdr_physics_object* Other)> Callback)
{
  CollideCallback = Callback;
  return;
}

double gdr::gdr_physics_object::GetMass(void)
{
  if (IsStatic)
    return 0;

  physx::PxRigidBody *BodyReal = Body->is<physx::PxRigidBody>();
  return BodyReal->getMass();
}

void gdr::gdr_physics_object::ApplyForce(float3 F)
{
  if (IsStatic)
    return;
  physx::PxRigidBody *BodyReal = Body->is<physx::PxRigidBody>();
  BodyReal->addForce({ F[0], F[1], F[2] });
  return;
}
gdr::gdr_physics_object::~gdr_physics_object()
{
    if (Body != nullptr)
    {
        Body->release();
        Body = nullptr;
    }
    if (Material != nullptr)
    {
        Material->release();
        Material = nullptr;
    }
}
