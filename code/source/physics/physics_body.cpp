#include "p_header.h"

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

gdr_index gdr::physic_body::GetParent() const
{
    return ParentIndex;
}

void gdr::physic_body::SetParent(gdr_index index)
{
    ParentIndex = index;
}


mth::matr4f gdr::physic_body::GetTransform(void) const
{
  return mth::matr4f::BuildTransform({1, 1, 1}, InterpolatedRot, InterpolatedPos);
}

void gdr::physic_body::ChangeDensity(float Density)
{
  if (IsStatic)
    return;
  physx::PxRigidBody *BodyReal = PhysxBody->is<physx::PxRigidBody>();
  physx::PxRigidBodyExt::updateMassAndInertia(*BodyReal, Density);
}

void gdr::physic_body::ChangeRotation(mth::matr4f Rot)
{
  physx::PxRigidBody *BodyReal = PhysxBody->is<physx::PxRigidBody>();
  physx::PxVec3 Pos = BodyReal->getGlobalPose().p;
  float arr[9] = {
      Rot[0][0], Rot[0][1], Rot[0][2],
      Rot[1][0], Rot[1][1], Rot[1][2],
      Rot[2][0], Rot[2][1], Rot[2][2] };
  physx::PxMat33 RotM = physx::PxMat33(arr);
  BodyReal->setGlobalPose(physx::PxTransform(Pos, physx::PxQuat(RotM)));
}

void gdr::physic_body::AddVelocity(mth::vec3f Vel)
{
  if (IsStatic)
    return;
  physx::PxRigidBody *BodyReal = PhysxBody->is<physx::PxRigidBody>();
  
  physx::PxVec3 OldVelocity = BodyReal->getLinearVelocity();
  OldVelocity[0] += Vel[0];
  OldVelocity[1] += Vel[1];
  OldVelocity[2] += Vel[2];
  BodyReal->setLinearVelocity(OldVelocity);
  return;
}

void gdr::physic_body::SetVelocity(mth::vec3f Vel)
{
  if (IsStatic)
    return;
  physx::PxRigidBody* BodyReal = PhysxBody->is<physx::PxRigidBody>();

  physx::PxVec3 OldVelocity = BodyReal->getLinearVelocity();
  OldVelocity[0] = Vel[0];
  OldVelocity[1] = Vel[1];
  OldVelocity[2] = Vel[2];
  BodyReal->setLinearVelocity(OldVelocity);
  return;
}

float3 gdr::physic_body::GetVelocity(void) const
{
  if (IsStatic)
    return {0, 0, 0};
  physx::PxRigidBody* BodyReal = PhysxBody->is<physx::PxRigidBody>();
  physx::PxVec3 OldVelocity = BodyReal->getLinearVelocity();
  return {OldVelocity[0], OldVelocity[1], OldVelocity[2]};
}

void gdr::physic_body::SetPos(mth::vec3f Pos)
{
  if (IsStatic)
    return;
  physx::PxRigidBody *BodyReal = PhysxBody->is<physx::PxRigidBody>();
  physx::PxQuat Rot = BodyReal->getGlobalPose().q;
  BodyReal->setGlobalPose(physx::PxTransform({ Pos[0], Pos[1], Pos[2] }, Rot));
  return;
}
void gdr::physic_body::Stop(void)
{
  if (IsStatic)
    return;

  physx::PxRigidBody *BodyReal = PhysxBody->is<physx::PxRigidBody>();
  BodyReal->setLinearVelocity(physx::PxVec3(0));
  BodyReal->setAngularVelocity(physx::PxVec3(0));
  return;
}

void gdr::physic_body::SetCollideCallback(std::function<void(gdr_index Me, gdr_index Other)> Callback)
{
  CollideCallback = Callback;
  return;
}

double gdr::physic_body::GetMass(void) const
{
  if (IsStatic)
    return 0;

  physx::PxRigidBody *BodyReal = PhysxBody->is<physx::PxRigidBody>();
  return BodyReal->getMass();
}

void gdr::physic_body::ApplyForce(mth::vec3f F)
{
  if (IsStatic)
    return;
  physx::PxRigidBody *BodyReal = PhysxBody->is<physx::PxRigidBody>();
  BodyReal->addForce({ F[0], F[1], F[2] });
  return;
}