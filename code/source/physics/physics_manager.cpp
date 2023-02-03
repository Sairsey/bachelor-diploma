#include "p_header.h"
#ifdef min
#undef min
#endif
#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"
#ifndef min
#define min(a,b)            (((a) < (b)) ? (a) : (b))
#endif

#include <thread>

#define PX_RELEASE(x)	if(x)	{ x->release(); x = NULL;	}

#ifdef _DEBUG
#pragma comment(lib, "PhysX/lib_debug/PhysXCharacterKinematic_static_64.lib")
#pragma comment(lib, "PhysX/lib_debug/PhysXCooking_64.lib")
#pragma comment(lib, "PhysX/lib_debug/PhysXExtensions_static_64.lib")
#pragma comment(lib, "PhysX/lib_debug/PhysXTask_static_64.lib")
#pragma comment(lib, "PhysX/lib_debug/PhysXVehicle_static_64.lib")
#pragma comment(lib, "PhysX/lib_debug/LowLevel_static_64.lib")
#pragma comment(lib, "PhysX/lib_debug/LowLevelAABB_static_64.lib")
#pragma comment(lib, "PhysX/lib_debug/LowLevelDynamics_static_64.lib")
#pragma comment(lib, "PhysX/lib_debug/PhysX_64.lib")
#pragma comment(lib, "PhysX/lib_debug/PhysXPvdSDK_static_64.lib")
#pragma comment(lib, "PhysX/lib_debug/PhysXFoundation_64.lib")
#pragma comment(lib, "PhysX/lib_debug/PhysXCommon_64.lib")
#else
#pragma comment(lib, "PhysX/lib_release/PhysXCharacterKinematic_static_64.lib")
#pragma comment(lib, "PhysX/lib_release/PhysXCooking_64.lib")
#pragma comment(lib, "PhysX/lib_release/PhysXExtensions_static_64.lib")
#pragma comment(lib, "PhysX/lib_release/PhysXTask_static_64.lib")
#pragma comment(lib, "PhysX/lib_release/PhysXVehicle_static_64.lib")
#pragma comment(lib, "PhysX/lib_release/LowLevel_static_64.lib")
#pragma comment(lib, "PhysX/lib_release/LowLevelAABB_static_64.lib")
#pragma comment(lib, "PhysX/lib_release/LowLevelDynamics_static_64.lib")
#pragma comment(lib, "PhysX/lib_release/PhysX_64.lib")
#pragma comment(lib, "PhysX/lib_release/PhysXPvdSDK_static_64.lib")
#pragma comment(lib, "PhysX/lib_release/PhysXFoundation_64.lib")
#pragma comment(lib, "PhysX/lib_release/PhysXCommon_64.lib")
#endif

physx::PxFilterFlags contactReportFilterShader(physx::PxFilterObjectAttributes attributes0, physx::PxFilterData filterData0,
  physx::PxFilterObjectAttributes attributes1, physx::PxFilterData filterData1,
  physx::PxPairFlags& pairFlags, const void* constantBlock, physx::PxU32 constantBlockSize)
{
  PX_UNUSED(attributes0);
  PX_UNUSED(attributes1);
  PX_UNUSED(filterData0);
  PX_UNUSED(filterData1);
  PX_UNUSED(constantBlockSize);
  PX_UNUSED(constantBlock);

  // all initial and persisting reports for everything, with per-point data
  pairFlags = physx::PxPairFlag::eSOLVE_CONTACT | physx::PxPairFlag::eDETECT_DISCRETE_CONTACT
    | physx::PxPairFlag::eNOTIFY_TOUCH_FOUND
    | physx::PxPairFlag::eNOTIFY_TOUCH_PERSISTS
    | physx::PxPairFlag::eNOTIFY_CONTACT_POINTS;
  return physx::PxFilterFlag::eDEFAULT;
}

class ContactReportCallback : public physx::PxSimulationEventCallback
{
private:
  gdr::physics_manager* Phys;
public:
  void setPhys(gdr::physics_manager* phys)
  {
    Phys = phys;
  }

  void onConstraintBreak(physx::PxConstraintInfo* constraints, physx::PxU32 count) { PX_UNUSED(constraints); PX_UNUSED(count); }
  void onWake(physx::PxActor** actors, physx::PxU32 count) { PX_UNUSED(actors); PX_UNUSED(count); }
  void onSleep(physx::PxActor** actors, physx::PxU32 count) { PX_UNUSED(actors); PX_UNUSED(count); }
  void onTrigger(physx::PxTriggerPair* pairs, physx::PxU32 count) { PX_UNUSED(pairs); PX_UNUSED(count); }
  void onAdvance(const physx::PxRigidBody* const*, const physx::PxTransform*, const physx::PxU32) {}
  void onContact(const physx::PxContactPairHeader& pairHeader, const physx::PxContactPair* pairs, physx::PxU32 nbPairs)
  {
    for (physx::PxU32 i = 0; i < nbPairs; i++)
    {
      const physx::PxContactPair& cp = pairs[i];

      if (cp.events & physx::PxPairFlag::eNOTIFY_TOUCH_FOUND)
      {
        physx::PxActor* MyActor = pairHeader.actors[0];
        physx::PxActor* OtherActor = pairHeader.actors[1];
        if (MyActor->userData && OtherActor->userData)
        {
          gdr_index ObjectIndex = (gdr_index)MyActor->userData;
          gdr_index OtherObjectIndex = (gdr_index)OtherActor->userData;
          gdr::physic_body* Object = Phys->IsExist(ObjectIndex) ? &Phys->GetEditable(ObjectIndex) : nullptr;
          gdr::physic_body* OtherObject = Phys->IsExist(OtherObjectIndex)? &Phys->GetEditable(OtherObjectIndex) : nullptr;
          if (Object)
            Object->CollideCallback(ObjectIndex, OtherObjectIndex);
          if (OtherObject)
            OtherObject->CollideCallback(OtherObjectIndex, ObjectIndex);
        }
      }
    }
  }
};
ContactReportCallback gContactReportCallback;

gdr::physics_manager::physics_manager(engine * Eng) : resource_pool_subsystem(Eng), Engine(Eng)
{
  static const bool IsVisualDebugger = false;

  //init SDK
  Foundation = PxCreateFoundation(PX_PHYSICS_VERSION, Allocator, ErrorCallback);
  //init Visual Debugger if needed
  if (IsVisualDebugger)
  {
    PhysVisualDebugger = physx::PxCreatePvd(*Foundation); // create PhysX Visual debugger instance
    physx::PxPvdTransport* transport = physx::PxDefaultPvdSocketTransportCreate("127.0.0.1", 5425, 10); // Set up Transport protocol
    PhysVisualDebugger->connect(*transport, physx::PxPvdInstrumentationFlag::eALL); // Connect one to another
  }
  else
    PhysVisualDebugger = nullptr;

  //Init Physics
  PhysX = PxCreatePhysics(PX_PHYSICS_VERSION, *Foundation, physx::PxTolerancesScale(), true, PhysVisualDebugger);
  PxInitExtensions(*PhysX, PhysVisualDebugger);

  //Init Cooking
  Cooking = PxCreateCooking(PX_PHYSICS_VERSION, *Foundation, PhysX->getTolerancesScale());

  // Descript Scene
  physx::PxSceneDesc sceneDesc(PhysX->getTolerancesScale());

  // set gravity
  sceneDesc.gravity = physx::PxVec3(0.0f, -9.80665f, 0.0f);

  // set Number of threads to compute 
  const auto processor_count = std::thread::hardware_concurrency(); // can return 0 if cannot determine
  Dispatcher = physx::PxDefaultCpuDispatcherCreate(processor_count > 1 ? processor_count - 2 : 1);
  sceneDesc.cpuDispatcher = Dispatcher;
  sceneDesc.filterShader = physx::PxDefaultSimulationFilterShader;
  
  sceneDesc.flags |= physx::PxSceneFlag::eENABLE_PCM;
  sceneDesc.broadPhaseType = physx::PxBroadPhaseType::eSAP;
  sceneDesc.filterShader = contactReportFilterShader;
  sceneDesc.simulationEventCallback = &gContactReportCallback;

  gContactReportCallback.setPhys(this);

  //finally create Scene
  Scene = PhysX->createScene(sceneDesc);
  physx::PxPvdSceneClient* pvdClient = Scene->getScenePvdClient();
  if (pvdClient)
  {
    pvdClient->setScenePvdFlag(physx::PxPvdSceneFlag::eTRANSMIT_CONSTRAINTS, true);
    pvdClient->setScenePvdFlag(physx::PxPvdSceneFlag::eTRANSMIT_CONTACTS, true);
    pvdClient->setScenePvdFlag(physx::PxPvdSceneFlag::eTRANSMIT_SCENEQUERIES, true);
  }

  physx::PxMaterial* Material = PhysX->createMaterial(1.f, 1.f, 0.1f);
  groundPlane = PxCreatePlane(*PhysX, physx::PxPlane(0, 1, 0, 0), *Material);
  groundPlane->userData = (void*)-1;
  Scene->addActor(*groundPlane);

  Material->release();
  resource_pool_subsystem::Add(); // Add one for ground plane
  Scene->simulate(PHYSICS_TICK);
}

gdr_index gdr::physics_manager::AddDynamicSphere(float Radius, gdr::physic_material Material)
{
  gdr_index NewSphereIndex = resource_pool_subsystem::Add();
  physic_body &NewBody = resource_pool_subsystem::GetEditable(NewSphereIndex);
  NewBody.Material = Material;
  NewBody.PhysxBody = PhysX->createRigidDynamic(physx::PxTransform(physx::PxIdentity));
  NewBody.PhysxMaterial = PhysX->createMaterial(Material.StaticFriction, Material.DynamicFriction, Material.Restitution);
  physx::PxShape* SphereShape = physx::PxRigidActorExt::createExclusiveShape(*NewBody.PhysxBody, physx::PxSphereGeometry(Radius), *NewBody.PhysxMaterial);
  NewBody.PhysxBody->userData = (void *)NewSphereIndex;
  Scene->addActor(*NewBody.PhysxBody);
  NewBody.IsStatic = false;
  return NewSphereIndex;
}

gdr_index gdr::physics_manager::AddDynamicCapsule(float Radius, float HalfHeight, physic_material Material)
{
  gdr_index NewCapsuleIndex = resource_pool_subsystem::Add();
  physic_body& NewBody = resource_pool_subsystem::GetEditable(NewCapsuleIndex);
  NewBody.Material = Material;
  NewBody.PhysxBody = PhysX->createRigidDynamic(physx::PxTransform(physx::PxIdentity));
  NewBody.PhysxMaterial = PhysX->createMaterial(Material.StaticFriction, Material.DynamicFriction, Material.Restitution);
  
  physx::PxTransform relativePose(physx::PxQuat(physx::PxHalfPi, { 0, 0, 1 }));
  physx::PxShape* CapsuleShape = PhysX->createShape(physx::PxCapsuleGeometry(Radius, HalfHeight), *NewBody.PhysxMaterial);
  CapsuleShape->setLocalPose(relativePose);
  NewBody.PhysxBody->attachShape(*CapsuleShape);
  CapsuleShape->release();

  NewBody.PhysxBody->userData = (void*)NewCapsuleIndex;
  Scene->addActor(*NewBody.PhysxBody);
  NewBody.IsStatic = false;
  return NewCapsuleIndex;
} 

gdr_index gdr::physics_manager::AddStaticMesh(model_import_data ImportModel, physic_material Material)
{
  gdr_index NewStaticMeshIndex = resource_pool_subsystem::Add();
  physic_body& NewBody = resource_pool_subsystem::GetEditable(NewStaticMeshIndex);
  NewBody.Material = Material;
  NewBody.PhysxBody = PhysX->createRigidStatic(physx::PxTransform(physx::PxIdentity));
  NewBody.PhysxMaterial = PhysX->createMaterial(Material.StaticFriction, Material.DynamicFriction, Material.Restitution);

  for (int node_index = 0; node_index < ImportModel.HierarchyNodes.size(); node_index++)
    if (ImportModel.HierarchyNodes[node_index].Type == gdr_hier_node_type::mesh)
    {
      import_model_node &MeshNode = ImportModel.HierarchyNodes[node_index];
      const unsigned CutSize = 65536;
      unsigned  CutCount = ((unsigned)MeshNode.Indices.size() / 3 + CutSize - 1) / CutSize;

      mth::matr4f myTransform = ImportModel.GetTransform(node_index);

      std::vector<mth::vec3f> Vert;
      Vert.resize(MeshNode.Vertices.size());
      for (int i = 0; i < Vert.size(); i++)
        Vert[i] = MeshNode.Vertices[i].Pos * myTransform;

      for (unsigned i = 0; i < CutCount; i++)
      {
        physx::PxTriangleMeshDesc meshDesc;
        meshDesc.points.count = (int)Vert.size();
        meshDesc.points.stride = sizeof(mth::vec3f);
        meshDesc.points.data = Vert.data();

        meshDesc.triangles.count = min(CutSize, (UINT32)(MeshNode.Indices.size() / 3 - i * CutSize));
        meshDesc.triangles.stride = 3 * sizeof(UINT32);
        meshDesc.triangles.data = MeshNode.Indices.data() + i * 3 * CutSize;

        physx::PxDefaultMemoryOutputStream writeBuffer;
        physx::PxTriangleMeshCookingResult::Enum result;
        bool status = Cooking->cookTriangleMesh(meshDesc, writeBuffer, &result);
        GDR_ASSERT(status);
        physx::PxDefaultMemoryInputData readBuffer(writeBuffer.getData(), writeBuffer.getSize());
        physx::PxTriangleMesh* Model = PhysX->createTriangleMesh(readBuffer);
        physx::PxShape* Shape = PhysX->createShape((physx::PxTriangleMeshGeometry(Model, physx::PxMeshScale(), physx::PxMeshGeometryFlag::eDOUBLE_SIDED)), *NewBody.PhysxMaterial);
        NewBody.PhysxBody->attachShape(*Shape);
        Shape->userData = (void*)NewStaticMeshIndex;
        Shape->release();
      }
    }

  NewBody.PhysxBody->userData = (void*)NewStaticMeshIndex;
  Scene->addActor(*NewBody.PhysxBody);
  NewBody.IsStatic = true;
  return NewStaticMeshIndex;
}

gdr_index gdr::physics_manager::AddDynamicMesh(model_import_data ImportModel, physic_material Material)
{
  gdr_index NewDynamicMeshIndex = resource_pool_subsystem::Add();
  physic_body& NewBody = resource_pool_subsystem::GetEditable(NewDynamicMeshIndex);
  NewBody.Material = Material;
  NewBody.PhysxBody = PhysX->createRigidStatic(physx::PxTransform(physx::PxIdentity));
  NewBody.PhysxMaterial = PhysX->createMaterial(Material.StaticFriction, Material.DynamicFriction, Material.Restitution);
  
  std::vector<mth::vec3f> Vert;
  for (int node_index = 0; node_index < ImportModel.HierarchyNodes.size(); node_index++)
    if (ImportModel.HierarchyNodes[node_index].Type == gdr_hier_node_type::mesh)
    {
      size_t StartIndex = Vert.size();
      Vert.resize(Vert.size() + ImportModel.HierarchyNodes[node_index].Vertices.size());
      
      mth::matr4f myTransform = ImportModel.GetTransform(node_index);

      for (size_t i = StartIndex; i < Vert.size(); i++)
        Vert[i] = ImportModel.HierarchyNodes[node_index].Vertices[i - StartIndex].Pos * myTransform;
    }

  {
    physx::PxConvexMeshDesc convexDesc;
    convexDesc.points.count = Vert.size();
    convexDesc.points.stride = sizeof(mth::vec3f);
    convexDesc.points.data = Vert.data();
    convexDesc.flags = physx::PxConvexFlag::eCOMPUTE_CONVEX;
    convexDesc.vertexLimit = 64;

    physx::PxDefaultMemoryOutputStream buf;
    bool status = Cooking->cookConvexMesh(convexDesc, buf);
    GDR_ASSERT(status);
    physx::PxDefaultMemoryInputData readBuffer(buf.getData(), buf.getSize());
    physx::PxConvexMesh* Model = PhysX->createConvexMesh(readBuffer);

    physx::PxShape* Shape = PhysX->createShape(physx::PxConvexMeshGeometry(Model), *NewBody.PhysxMaterial);
    NewBody.PhysxBody->attachShape(*Shape);
    Shape->userData = (void*)NewDynamicMeshIndex;
    Shape->release();
  }
  
  NewBody.PhysxBody->userData = (void*)NewDynamicMeshIndex;
  Scene->addActor(*NewBody.PhysxBody);
  NewBody.IsStatic = true;
  return NewDynamicMeshIndex;
}

void gdr::physics_manager::BeforeRemoveJob(gdr_index index)
{
  if (IsExist(index))
  {
    ToDelete.push_back(Get(index));
  }
}

void gdr::physics_manager::Update(float DeltaTime)
{
  SimulationDeltaTime += DeltaTime;

  if (!IsThrottle && SimulationDeltaTime > 3 * PHYSICS_TICK)
  {
      OutputDebugStringA("PHYSICS_DELTA_TIME IS TOO BIG. ENGINE PAUSED\n");
      IsThrottle = true;
  }

  Engine->SetPhysPause(IsThrottle);

  if ((IsThrottle || SimulationDeltaTime > PHYSICS_TICK) && Scene->fetchResults(false))
  {
    Engine->SetPause(false);
    PROFILE_CPU_BEGIN("Physics next frame");
    // delete objects then we are not simulating
    for (const auto& i : ToDelete)
    {
      i.PhysxBody->release();
      i.PhysxMaterial->release();
    }
    ToDelete.clear();

    SimulationDeltaTime -= PHYSICS_TICK;
    SimulationDeltaTime = max(SimulationDeltaTime, 0);
    if (SimulationDeltaTime == 0)
        IsThrottle = false;

    {
        PROFILE_CPU_BEGIN("Simulate");
        Scene->simulate(PHYSICS_TICK);
        PROFILE_CPU_END();
    }

    for (gdr_index i = 1; i < AllocatedSize(); i++)
      if (IsExist(i))
      {
        physx::PxTransform Trans = Get(i).PhysxBody->getGlobalPose();
        GetEditable(i).PrevTickPos = Get(i).IsCreated ? mth::vec3f{ Trans.p.x, Trans.p.y, Trans.p.z } : Get(i).NextTickPos;
        GetEditable(i).PrevTickRot = Get(i).IsCreated ? mth::vec4f{ Trans.q.x, Trans.q.y, Trans.q.z, Trans.q.w } : Get(i).NextTickRot;
        GetEditable(i).NextTickPos = { Trans.p.x, Trans.p.y, Trans.p.z };
        GetEditable(i).NextTickRot = { Trans.q.x, Trans.q.y, Trans.q.z, Trans.q.w };
        GetEditable(i).IsCreated = false;
      }
    PROFILE_CPU_END();
  }

  float interpolParam = (SimulationDeltaTime) / (PHYSICS_TICK);

  interpolParam = max(0, min(1, interpolParam));

  for (gdr_index i = 1; i < AllocatedSize(); i++)
    if (IsExist(i))
    {
      GetEditable(i).InterpolatedPos = Get(i).PrevTickPos * (1 - interpolParam) + Get(i).NextTickPos * (interpolParam);
      GetEditable(i).InterpolatedRot = Get(i).PrevTickRot.slerp(Get(i).NextTickRot, interpolParam);
    }
}

bool gdr::physics_manager::Raycast(mth::vec3f Org, mth::vec3f Dir, float MaxLength, std::vector<gdr::ray_intersect>& Output)
{
  std::vector<ray_intersect> Objects;
  const physx::PxU32 bufferSize = 256;        // [in] size of 'hitBuffer'
  physx::PxRaycastHit hitBuffer[bufferSize];  // [out] User provided buffer for results
  physx::PxRaycastBuffer buf(hitBuffer, bufferSize); // [out] Raycast results

  // Raycast against all static & dynamic objects (no filtering)
  // The main result from this call is the closest hit, stored in the 'hit.block' structure
  bool status = Scene->raycast({ Org[0], Org[1], Org[2] }, { Dir[0], Dir[1], Dir[2] }, MaxLength, buf);

  if (status)
  {
    for (uint32_t i = 0; i < buf.nbTouches; i++)
      if (buf.touches[i].actor->userData)
      {
        gdr_index ObjectIndex = (gdr_index)buf.touches[i].actor->userData;
        ray_intersect A;
        A.Index = ObjectIndex;
        A.Distance = buf.touches[i].distance;
        A.Position = { buf.touches[i].position.x, buf.touches[i].position.y, buf.touches[i].position.z };
        Objects.push_back(A);
      }
  }
  return status;
}

gdr::physics_manager::~physics_manager()
{
  PxCloseExtensions();
  PX_RELEASE(Cooking);
  PX_RELEASE(Scene);
  PX_RELEASE(PhysX);
  if (PhysVisualDebugger)
  {
    physx::PxPvdTransport* transport = PhysVisualDebugger->getTransport();
    PhysVisualDebugger->release();
    PhysVisualDebugger = NULL;
    PX_RELEASE(transport);
  }
  PX_RELEASE(Foundation);
}