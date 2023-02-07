#include "p_header.h"

mth::matr4f gdr::physic_body::GetTransform(void) const
{
  return mth::matr4f::BuildTransform({ 1, 1, 1 }, InterpolatedState.Rot, InterpolatedState.Pos);
}

float gdr::physic_body::GetMass(void) const
{
  if (IsStatic)
    return 0;

  physx::PxRigidBody* BodyReal = PhysxBody->is<physx::PxRigidBody>();
  return BodyReal->getMass();
}

gdr_index gdr::physic_body::GetParent() const
{
  return ParentIndex;
}

mth::vec3f gdr::physic_body::GetVel(void) const
{
  return NextTickState.Vel;
}

mth::vec3f gdr::physic_body::GetPos(void) const
{
  return InterpolatedState.Pos;
}

mth::vec4f gdr::physic_body::GetRot(void) const
{
  return InterpolatedState.Rot;
}

void gdr::physic_body::Stop(void)
{
  if (IsStatic)
    return;

  physx::PxRigidBody* BodyReal = PhysxBody->is<physx::PxRigidBody>();
  BodyReal->setLinearVelocity(physx::PxVec3(0));
  BodyReal->setAngularVelocity(physx::PxVec3(0));
  return;
}

void gdr::physic_body::AddForce(mth::vec3f F)
{
  if (IsStatic)
    return;
  physx::PxRigidBody* BodyReal = PhysxBody->is<physx::PxRigidBody>();
  BodyReal->addForce({ F[0], F[1], F[2] });
  return;
}

void gdr::physic_body::AddVel(mth::vec3f Vel)
{
  if (IsStatic)
    return;
  physx::PxRigidBody* BodyReal = PhysxBody->is<physx::PxRigidBody>();

  BodyReal->addForce({ Vel[0], Vel[1], Vel[2] }, physx::PxForceMode::eVELOCITY_CHANGE);
  return;
}

void gdr::physic_body::SetVel(mth::vec3f Vel)
{
  if (IsStatic)
    return;
  physx::PxRigidBody* BodyReal = PhysxBody->is<physx::PxRigidBody>();
  BodyReal->setLinearVelocity({ Vel[0], Vel[1], Vel[2] });
  return;
}

void gdr::physic_body::SetPos(mth::vec3f Pos)
{
  if (IsStatic)
    return;
  physx::PxRigidBody* BodyReal = PhysxBody->is<physx::PxRigidBody>();
  physx::PxQuat Rot = { NextTickState.Rot[0], NextTickState.Rot[1], NextTickState.Rot[2], NextTickState.Rot[3] };
  BodyReal->setGlobalPose(physx::PxTransform({ Pos[0], Pos[1], Pos[2] }, Rot));
  return;
}

void gdr::physic_body::SetRot(mth::matr4f Rot)
{
  physx::PxRigidBody* BodyReal = PhysxBody->is<physx::PxRigidBody>();
  physx::PxVec3 Pos = { NextTickState.Pos[0], NextTickState.Pos[1], NextTickState.Pos[2] };
  float arr[9] = {
      Rot[0][0], Rot[0][1], Rot[0][2],
      Rot[1][0], Rot[1][1], Rot[1][2],
      Rot[2][0], Rot[2][1], Rot[2][2] };
  physx::PxMat33 RotM = physx::PxMat33(arr);
  BodyReal->setGlobalPose(physx::PxTransform(Pos, physx::PxQuat(RotM)));
}

void gdr::physic_body::SetRot(mth::vec4f Rot)
{
  physx::PxRigidBody* BodyReal = PhysxBody->is<physx::PxRigidBody>();
  physx::PxVec3 Pos = { NextTickState.Pos[0], NextTickState.Pos[1], NextTickState.Pos[2] };
  BodyReal->setGlobalPose(physx::PxTransform(Pos, { Rot[0], Rot[1], Rot[2], Rot[3] }));
}

void gdr::physic_body::SetParent(gdr_index index)
{
  ParentIndex = index;
}

void gdr::physic_body::ChangeDensity(float Density)
{
  if (IsStatic)
    return;
  physx::PxRigidBody* BodyReal = PhysxBody->is<physx::PxRigidBody>();
  physx::PxRigidBodyExt::updateMassAndInertia(*BodyReal, Density);
}

void gdr::physic_body::ToggleRotation(void)
{
  physx::PxRigidDynamic *BodyReal = PhysxBody->is<physx::PxRigidDynamic>();
  IsLockedRotation = !IsLockedRotation;
  physx::PxRigidDynamicLockFlags flags;
  if (IsLockedRotation)
    flags = flags | physx::PxRigidDynamicLockFlag::eLOCK_ANGULAR_X | physx::PxRigidDynamicLockFlag::eLOCK_ANGULAR_Y | physx::PxRigidDynamicLockFlag::eLOCK_ANGULAR_Z;
  if (IsLockedTranslation)
    flags = flags | physx::PxRigidDynamicLockFlag::eLOCK_LINEAR_X | physx::PxRigidDynamicLockFlag::eLOCK_LINEAR_Y | physx::PxRigidDynamicLockFlag::eLOCK_LINEAR_Z;
  BodyReal->setRigidDynamicLockFlags(flags);
}

void gdr::physic_body::ToggleTranslation(void)
{
  physx::PxRigidDynamic *BodyReal = PhysxBody->is<physx::PxRigidDynamic>();
  IsLockedTranslation = !IsLockedTranslation;
  physx::PxRigidDynamicLockFlags flags;
  if (IsLockedRotation)
    flags = flags | physx::PxRigidDynamicLockFlag::eLOCK_ANGULAR_X | physx::PxRigidDynamicLockFlag::eLOCK_ANGULAR_Y | physx::PxRigidDynamicLockFlag::eLOCK_ANGULAR_Z;
  if (IsLockedTranslation)
    flags = flags | physx::PxRigidDynamicLockFlag::eLOCK_LINEAR_X | physx::PxRigidDynamicLockFlag::eLOCK_LINEAR_Y | physx::PxRigidDynamicLockFlag::eLOCK_LINEAR_Z;
  BodyReal->setRigidDynamicLockFlags(flags);
}

void gdr::physic_body::SetCollideCallback(std::function<void(gdr_index Me, gdr_index Other)> Callback)
{
  CollideCallback = Callback;
  return;
}
