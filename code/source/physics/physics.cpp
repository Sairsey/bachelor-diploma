#include "p_header.h"

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

namespace physx
{
  PxFilterFlags contactReportFilterShader(PxFilterObjectAttributes attributes0, PxFilterData filterData0,
    PxFilterObjectAttributes attributes1, PxFilterData filterData1,
    PxPairFlags& pairFlags, const void* constantBlock, PxU32 constantBlockSize)
  {
    PX_UNUSED(attributes0);
    PX_UNUSED(attributes1);
    PX_UNUSED(filterData0);
    PX_UNUSED(filterData1);
    PX_UNUSED(constantBlockSize);
    PX_UNUSED(constantBlock);

    // all initial and persisting reports for everything, with per-point data
    pairFlags = PxPairFlag::eSOLVE_CONTACT | PxPairFlag::eDETECT_DISCRETE_CONTACT
      | PxPairFlag::eNOTIFY_TOUCH_FOUND
      | PxPairFlag::eNOTIFY_TOUCH_PERSISTS
      | PxPairFlag::eNOTIFY_CONTACT_POINTS; 
    return PxFilterFlag::eDEFAULT;
  }
}

class ContactReportCallback : public physx::PxSimulationEventCallback
{
private:
    gdr::physics* Phys;
public:
    void setPhys(gdr::physics* phys)
    {
        Phys = phys;
    }

  void onConstraintBreak(physx::PxConstraintInfo* constraints, physx::PxU32 count) { PX_UNUSED(constraints); PX_UNUSED(count); }
  void onWake(physx::PxActor** actors, physx::PxU32 count) { PX_UNUSED(actors); PX_UNUSED(count); }
  void onSleep(physx::PxActor** actors, physx::PxU32 count) { PX_UNUSED(actors); PX_UNUSED(count); }
  void onTrigger(physx::PxTriggerPair* pairs, physx::PxU32 count) { PX_UNUSED(pairs); PX_UNUSED(count); }
  void onAdvance(const physx::PxRigidBody*const*, const physx::PxTransform*, const physx::PxU32) {}
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
            gdr::gdr_physics_object& Object = Phys->GetPhysObject((gdr::gdr_index)MyActor->userData);
            gdr::gdr_physics_object& OtherObject = Phys->GetPhysObject((gdr::gdr_index)OtherActor->userData);
            Object.CollideCallback(&Object, &OtherObject);
        }
      }
    }
  }
};

ContactReportCallback gContactReportCallback;

gdr::physics::physics(bool IsVisualDebugger)
{
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
        PhysVisualDebugger = NULL;
    //Init Physics
    Physics = PxCreatePhysics(PX_PHYSICS_VERSION, *Foundation, physx::PxTolerancesScale(), true, PhysVisualDebugger);
    PxInitExtensions(*Physics, PhysVisualDebugger);

    //Init Cooking
    Cooking = PxCreateCooking(PX_PHYSICS_VERSION, *Foundation, Physics->getTolerancesScale());
    // Add Cuda support
    physx::PxCudaContextManagerDesc cudaContextManagerDesc;
    CudaContextManager = PxCreateCudaContextManager(*Foundation, cudaContextManagerDesc, PxGetProfilerCallback());

    // Descript Scene
    physx::PxSceneDesc sceneDesc(Physics->getTolerancesScale());
    // set gravity
    sceneDesc.gravity = physx::PxVec3(0.0f, -9.81f, 0.0f);
    // set Number of threads to compute 
    const auto processor_count = std::thread::hardware_concurrency(); // can return 0 if cannot determine
    Dispatcher = physx::PxDefaultCpuDispatcherCreate(processor_count > 1 ? processor_count - 2 : 1);
    sceneDesc.cpuDispatcher = Dispatcher;
    sceneDesc.filterShader = physx::PxDefaultSimulationFilterShader;
    sceneDesc.cudaContextManager = CudaContextManager;

    sceneDesc.flags |= physx::PxSceneFlag::eENABLE_GPU_DYNAMICS;
    sceneDesc.broadPhaseType = physx::PxBroadPhaseType::eGPU;
    sceneDesc.filterShader = physx::contactReportFilterShader;
    sceneDesc.simulationEventCallback = &gContactReportCallback;

    gContactReportCallback.setPhys(this);

    //finally create Scene
    Scene = Physics->createScene(sceneDesc);
    physx::PxPvdSceneClient* pvdClient = Scene->getScenePvdClient();
    if (pvdClient)
    {
        pvdClient->setScenePvdFlag(physx::PxPvdSceneFlag::eTRANSMIT_CONSTRAINTS, true);
        pvdClient->setScenePvdFlag(physx::PxPvdSceneFlag::eTRANSMIT_CONTACTS, true);
        pvdClient->setScenePvdFlag(physx::PxPvdSceneFlag::eTRANSMIT_SCENEQUERIES, true);
    }

    physx::PxMaterial* Material = Physics->createMaterial(0.5f, 0.5f, 0.6f);
    groundPlane = PxCreatePlane(*Physics, physx::PxPlane(0, 1, 0, 0), *Material);
    Scene->addActor(*groundPlane);

    Material->release();
}

std::vector<gdr::ray_intersect> gdr::physics::Raycast(mth::vec3f Org, mth::vec3f Dir, float MaxLength)
{
    std::vector<ray_intersect> Objects;
    const physx::PxU32 bufferSize = 256;        // [in] size of 'hitBuffer'
    physx::PxRaycastHit hitBuffer[bufferSize];  // [out] User provided buffer for results
    physx::PxRaycastBuffer buf(hitBuffer, bufferSize); // [out] Raycast results

    // Raycast against all static & dynamic objects (no filtering)
    // The main result from this call is the closest hit, stored in the 'hit.block' structure
    bool status = Scene->raycast({ Org[0], Org[1], Org[2] }, { Dir[0], Dir[1], Dir[2] }, MaxLength, buf);

    for (uint32_t i = 0; i < buf.nbTouches; i++)
        if (buf.touches[i].actor->userData)
        {
            gdr_index ObjectIndex = (gdr_index)buf.touches[i].actor->userData;
            ray_intersect A;
            A.PhysObjectIndex = ObjectIndex;
            A.Distance = buf.touches[i].distance;
            Objects.push_back(A);
        }
    return Objects;
}

void gdr::physics::Update(double DeltaTime)
{
    for (const auto& i : ForDelete)
    {
        delete ObjectsPool[i];
        ObjectsPool[i] = nullptr;
    }
    ForDelete.clear();
    Scene->simulate(DeltaTime);
    Scene->fetchResults(true);
}

gdr::gdr_index gdr::physics::NewDynamicSphere(physic_material Material, double Radius, std::string name)
{
    ObjectsPool.push_back(new gdr_physics_object()); // TODO - change on smart choosing
    gdr_physics_object &Obj = *ObjectsPool[ObjectsPool.size() - 1];
    Obj.Body = Physics->createRigidDynamic(physx::PxTransform(physx::PxIdentity));
    Obj.Material = Physics->createMaterial(Material.StaticFriction, Material.DynamicFriction, Material.Restitution);
    physx::PxShape* SphereShape = physx::PxRigidActorExt::createExclusiveShape(*Obj.Body, physx::PxSphereGeometry(Radius), *Obj.Material);
    Obj.MyIndex = ObjectsPool.size() - 1;
    Obj.Body->userData = (void*)Obj.MyIndex;
    Obj.Name = name;
    Scene->addActor(*Obj.Body);
    return Obj.MyIndex;
}

gdr::gdr_index gdr::physics::NewDynamicCapsule(physic_material Material, double Radius, double HalfHeight, std::string name)
{
    ObjectsPool.emplace_back(new gdr_physics_object()); // TODO - change on smart choosing
    gdr_physics_object& Obj = *ObjectsPool[ObjectsPool.size() - 1];
    Obj.Body = Physics->createRigidDynamic(physx::PxTransform(physx::PxIdentity));
    Obj.Material = Physics->createMaterial(Material.StaticFriction, Material.DynamicFriction, Material.Restitution);
    physx::PxTransform relativePose(physx::PxQuat(physx::PxHalfPi, { 0, 0, 1 }));
    physx::PxShape* CapsuleShape = Physics->createShape(physx::PxCapsuleGeometry(Radius, HalfHeight), *Obj.Material);
    CapsuleShape->setLocalPose(relativePose);
    Obj.Body->attachShape(*CapsuleShape);
    Obj.MyIndex = ObjectsPool.size() - 1;
    Obj.Name = name;
    CapsuleShape->release();
    Obj.Body->userData = (void*)Obj.MyIndex;
    Scene->addActor(*Obj.Body);
    return Obj.MyIndex;
}

gdr::gdr_index gdr::physics::NewStaticMesh(physic_material Material, std::vector<vertex> VertexBuffer, std::vector<UINT32> Index, std::string name)
{
    ObjectsPool.emplace_back(new gdr_physics_object()); // TODO - change on smart choosing
    gdr_physics_object& Obj = *ObjectsPool[ObjectsPool.size() - 1];
    Obj.Body = Physics->createRigidDynamic(physx::PxTransform(physx::PxIdentity));
    Obj.Material = Physics->createMaterial(Material.StaticFriction, Material.DynamicFriction, Material.Restitution);
    {
        const unsigned CutSize = 65536;
        unsigned  CutCount = ((unsigned)Index.size() / 3 + CutSize - 1) / CutSize;

        std::vector<mth::vec3f> Vert;
        Vert.resize(VertexBuffer.size());
        for (int i = 0; i < Vert.size(); i++)
            Vert[i] = VertexBuffer[i].Pos;

        for (unsigned i = 0; i < CutCount; i++)
        {
            physx::PxTriangleMeshDesc meshDesc;
            meshDesc.points.count = (int)Vert.size();
            meshDesc.points.stride = sizeof(float3);
            meshDesc.points.data = Vert.data();

            meshDesc.triangles.count = min(CutSize, (UINT32)(Index.size() / 3 - i * CutSize));
            meshDesc.triangles.stride = 3 * sizeof(UINT32);
            meshDesc.triangles.data = Index.data() + i * 3 * CutSize;

            physx::PxDefaultMemoryOutputStream writeBuffer;
            physx::PxTriangleMeshCookingResult::Enum result;
            bool status = Cooking->cookTriangleMesh(meshDesc, writeBuffer, &result);
            if (!status)
                throw "Cannot setup model";
            physx::PxDefaultMemoryInputData readBuffer(writeBuffer.getData(), writeBuffer.getSize());
            physx::PxTriangleMesh* Model = Physics->createTriangleMesh(readBuffer);
            physx::PxShape* Shape = Physics->createShape((physx::PxTriangleMeshGeometry(Model, physx::PxMeshScale(), physx::PxMeshGeometryFlag::eDOUBLE_SIDED)), *Obj.Material);
            Obj.Body->attachShape(*Shape);
            Shape->release();
        }
    }
    Obj.MyIndex = ObjectsPool.size() - 1;
    Obj.Name = name;
    Obj.Body->userData = (void*)Obj.MyIndex;
    Scene->addActor(*Obj.Body);
    return Obj.MyIndex;
}

gdr::gdr_index gdr::physics::NewDynamicMesh(physic_material Material, std::vector<vertex> VertexBuffer, std::vector<UINT32> Index, std::string name)
{
    ObjectsPool.emplace_back(new gdr_physics_object()); // TODO - change on smart choosing
    gdr_physics_object& Obj = *ObjectsPool[ObjectsPool.size() - 1];
    Obj.Body = Physics->createRigidDynamic(physx::PxTransform(physx::PxIdentity));
    Obj.Material = Physics->createMaterial(Material.StaticFriction, Material.DynamicFriction, Material.Restitution);
    {
        if (VertexBuffer.size() == 0)
            return -1;

        std::vector<mth::vec3f> Vert;
        Vert.resize(VertexBuffer.size());
        for (int i = 0; i < Vert.size(); i++)
            Vert[i] = VertexBuffer[i].Pos;

        physx::PxConvexMeshDesc convexDesc;
        convexDesc.points.count = Vert.size();
        convexDesc.points.stride = sizeof(mth::vec3f);
        convexDesc.points.data = Vert.data();
        convexDesc.flags = physx::PxConvexFlag::eCOMPUTE_CONVEX;
        convexDesc.vertexLimit = 100;

        physx::PxDefaultMemoryOutputStream buf;
        if (!Cooking->cookConvexMesh(convexDesc, buf))
            throw "Cannot ConvexHull";
        physx::PxDefaultMemoryInputData readBuffer(buf.getData(), buf.getSize());
        physx::PxConvexMesh* Model = Physics->createConvexMesh(readBuffer);

        physx::PxShape* Shape = Physics->createShape(physx::PxConvexMeshGeometry(Model), *Obj.Material);
        Obj.Body->attachShape(*Shape);
        Shape->release();
    }
    Obj.MyIndex = ObjectsPool.size() - 1;
    Obj.Name = name;
    Obj.Body->userData = (void*)Obj.MyIndex;
    Scene->addActor(*Obj.Body);
    return Obj.MyIndex;
}

void gdr::physics::DeletePhysObject(gdr_index ToDelete)
{
    ForDelete.push_back(ToDelete);
}

gdr::physics::~physics()
{
    PxCloseExtensions();
    PX_RELEASE(Cooking);
    PX_RELEASE(Physics);
    if (PhysVisualDebugger)
    {
        physx::PxPvdTransport* transport = PhysVisualDebugger->getTransport();
        PhysVisualDebugger->release();
        PhysVisualDebugger = NULL;
        PX_RELEASE(transport);
    }
    PX_RELEASE(Foundation);
}
