#pragma once
#include "def.h"
#include "render/subsystems/geometry_support.h"
#include "render/subsystems/object_support.h"

namespace gdr
{
  //physic material 
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
    gdr_index PhysObjectIndex;
  };

  struct gdr_physics_object
  {
      physx::PxMaterial* Material = nullptr;
      physx::PxRigidActor* Body = nullptr;
      std::function<void(gdr_physics_object* Me, gdr_physics_object* Other)> CollideCallback = [](gdr_physics_object* Me, gdr_physics_object* Other) {return; };
      friend class physics;
      friend class ContactReportCallback;
      bool IsStatic = false;
      bool IsLockedTranslation = false;
      bool IsLockedRotation = false;
    public:
        std::string Name;
        gdr_index MyIndex;

        mth::matr4f GetTransform(void);
        double GetMass(void);
        void SetVelocity(float3 Vel);
        void AddVelocity(float3 Vel);
        float3 GetVelocity(void);
        void Stop(void);
        void ApplyForce(float3 F);
        void SetPos(float3 Pos);
        void ChangeDensity(float Density);
        void ToggleRotation(void);
        void ToggleTranslation(void);
        void ChangeRotation(mth::matr4f Rot);
        void SetCollideCallback(std::function<void(gdr_physics_object* Me, gdr_physics_object* Other)> Callback);
        ~gdr_physics_object();
  };

  // Physics class representation. Realisation can be found in physics.cpp
  class physics
  {
  private:
      std::vector<gdr_index> ForDelete;
      physx::PxFoundation* Foundation;                     // SDK
      physx::PxPhysics* Physics;                           // Physics repr
      physx::PxCudaContextManager* CudaContextManager;     // Cuda manager for GPU acceleration
      physx::PxDefaultCpuDispatcher* Dispatcher;           // Dispathcer to calculate physics
      physx::PxCooking* Cooking;                           // Cooker

      physx::PxDefaultAllocator Allocator;         // Default allocator for PhysX
      physx::PxDefaultErrorCallback ErrorCallback; // Default callback for PhysX
      physx::PxPvd* PhysVisualDebugger; // Visual Debugger if used  

      physx::PxScene* Scene;                               // Scene repr
      physx::PxRigidStatic* groundPlane;                   // Ground plane object
      std::vector<gdr_physics_object *> ObjectsPool;
  public:
      physics(bool IsVisualDebugger = true);
      std::vector<ray_intersect> Raycast(mth::vec3f Org, mth::vec3f Dir, float MaxLength);
      void Update(double DeltaTime);
      gdr_physics_object& GetPhysObject(gdr_index ObjectIndex) { return *ObjectsPool[ObjectIndex]; }
      size_t GetObjectsAmount(void) { return ObjectsPool.size(); }

      gdr_index NewDynamicSphere(physic_material Material, double Radius, std::string name);
      gdr_index NewDynamicCapsule(physic_material Material, double Radius, double HalfHeight, std::string name);
      gdr_index NewStaticMesh(physic_material Material, std::vector<vertex> VertexBuffer, std::vector<UINT32> Index, std::string name);
      gdr_index NewDynamicMesh(physic_material Material, std::vector<vertex> VertexBuffer, std::vector<UINT32> Index, std::string name);
      std::vector<gdr_index> NewStaticMeshAssimp(physic_material Material, std::string Filename);
      std::vector<gdr_index> NewDynamicMeshAssimp(physic_material Material, std::string Filename);
      void DeletePhysObject(gdr_index ToDelete);

      ~physics();
  };
}

