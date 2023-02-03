#pragma once
#include "def.h"

class ContactReportCallback;

namespace gdr
{
  const float PHYSICS_TICK = 1 / 30.0;
  struct physic_material
  {
    float StaticFriction;
    float DynamicFriction;
    float Restitution;
    physic_material(float S = 0.5f, float D = 0.5f, float R = 0.0f) : StaticFriction(S), DynamicFriction(D), Restitution(R)
    {
    }
  };
  
  struct ray_intersect
  {
    float Distance;
    mth::vec3f Position;
    gdr_index Index;
  };

  struct physic_body
  {
    private:
      friend class physics_manager;
      friend class ContactReportCallback;

      bool IsCreated = true;

      physx::PxRigidActor *PhysxBody;
      physx::PxMaterial* PhysxMaterial;
      physic_material Material;

      std::function<void(gdr_index Me, gdr_index Other)> CollideCallback = [](gdr_index Me, gdr_index Other) {return; };

      mth::vec3f PrevTickPos = { 0, 0, 0 };
      mth::vec3f InterpolatedPos;
      mth::vec3f NextTickPos = { 0, 0, 0 };

      mth::vec4f PrevTickRot = { 0, 0, 0, 1 };
      mth::vec4f InterpolatedRot;
      mth::vec4f NextTickRot = {0, 0, 0, 1};
      bool IsStatic = false;
      bool IsLockedTranslation = false;
      bool IsLockedRotation = false;
    public:

      mth::matr4f GetTransform(void) const;
      double GetMass(void) const;
      void SetVelocity(mth::vec3f Vel);
      void AddVelocity(mth::vec3f Vel);
      mth::vec3f GetVelocity(void) const;
      void Stop(void);
      void ApplyForce(mth::vec3f F);
      void SetPos(mth::vec3f Pos);
      void ChangeDensity(float Density);
      void ToggleRotation(void);
      void ToggleTranslation(void);
      void ChangeRotation(mth::matr4f Rot);
      void SetCollideCallback(std::function<void(gdr_index Me, gdr_index Other)> Callback);
  };

  class physics_manager : public resource_pool_subsystem<physic_body, 0>
  {
    private:
      void BeforeRemoveJob(gdr_index index) override;
      void UpdateGPUData(ID3D12GraphicsCommandList* pCommandList) {GDR_FAILED("Should not call this function!");};

      engine* Engine;

      physx::PxFoundation* Foundation;                     // SDK
      physx::PxPhysics* PhysX;                             // Physics repr
      physx::PxDefaultCpuDispatcher* Dispatcher;           // Dispathcer to calculate physics
      physx::PxCooking* Cooking;                           // Cooker

      physx::PxDefaultAllocator Allocator;                 // Default allocator for PhysX
      physx::PxDefaultErrorCallback ErrorCallback;         // Default callback for PhysX
      physx::PxPvd* PhysVisualDebugger;                    // Visual Debugger if used  

      physx::PxScene* Scene;                               // Scene repr
      physx::PxRigidStatic* groundPlane;                   // Ground plane object

      std::vector<physic_body> ToDelete;

      float SimulationDeltaTime = 0;
      bool IsThrottle;
    public:
      physics_manager(engine* Eng);

      gdr_index Add()
      {
        GDR_FAILED("IMPOSSIBLE TO PHYSICS BODY WITHOUT IMPORT DATA");
        return NONE_INDEX;
      }

      gdr_index AddDynamicSphere(float Radius, physic_material Material = physic_material());
      gdr_index AddDynamicCapsule(float Radius, float HalfHeight, physic_material Material = physic_material());
      gdr_index AddDynamicMesh(model_import_data ImportModel, physic_material Material = physic_material());
      gdr_index AddStaticMesh(model_import_data ImportModel, physic_material Material = physic_material());
      bool Raycast(mth::vec3f Org, mth::vec3f Dir, float MaxLength, std::vector<gdr::ray_intersect> &Output);
      void Update(float DeltaTime);

      ~physics_manager();
  };
}