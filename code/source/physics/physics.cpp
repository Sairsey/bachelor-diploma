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
            gdr::gdr_index ObjectIndex = (gdr::gdr_index)MyActor->userData - 1;
            gdr::gdr_index OtherObjectIndex = (gdr::gdr_index)OtherActor->userData - 1;
            gdr::gdr_physics_object *Object = ObjectIndex < Phys->GetObjectsAmount() ? &Phys->GetPhysObject(ObjectIndex) : nullptr;
            gdr::gdr_physics_object *OtherObject = OtherObjectIndex < Phys->GetObjectsAmount() ? &Phys->GetPhysObject(OtherObjectIndex) : nullptr;
            if (Object)
              Object->CollideCallback(Object, OtherObject);
            if (OtherObject)
              OtherObject->CollideCallback(OtherObject, Object);
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

    sceneDesc.flags |= physx::PxSceneFlag::eENABLE_GPU_DYNAMICS ;
    sceneDesc.flags |= physx::PxSceneFlag::eENABLE_PCM;
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

    physx::PxMaterial* Material = Physics->createMaterial(1.f, 1.f, 0.1f);
    groundPlane = PxCreatePlane(*Physics, physx::PxPlane(0, 1, 0, 0), *Material);
    groundPlane->userData = (void *)-1;
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
            gdr_index ObjectIndex = (gdr_index)buf.touches[i].actor->userData - 1;
            ray_intersect A;
            A.PhysObjectIndex = ObjectIndex;
            A.Distance = buf.touches[i].distance;
            Objects.push_back(A);
        }
    return Objects;
}

void gdr::physics::Update(double DeltaTime)
{
    static double counter = 0;
    static bool isFirst = true;
    for (const auto& i : ForDelete)
    {
        delete ObjectsPool[i];
        ObjectsPool[i] = nullptr;
    }
    ForDelete.clear();
    if (isFirst)
    {
      Scene->simulate(PHYSICS_TICK);
      isFirst = false;
      return;
    }
    counter += DeltaTime;

    PROFILE_CPU_BEGIN("Physics update")
    if (Scene->checkResults() && counter >= PHYSICS_TICK)
    {
      isPhysicsChanged = true;
      Scene->fetchResults(true);
      Scene->simulate(PHYSICS_TICK);
      counter = 0;
    }
    else
    {
      isPhysicsChanged = false;
    }
    PROFILE_CPU_END()
}

gdr::gdr_index gdr::physics::NewDynamicSphere(physic_material Material, double Radius, std::string name)
{
    ObjectsPool.push_back(new gdr_physics_object()); // TODO - change on smart choosing
    gdr_physics_object &Obj = *ObjectsPool[ObjectsPool.size() - 1];
    Obj.Body = Physics->createRigidDynamic(physx::PxTransform(physx::PxIdentity));
    Obj.Material = Physics->createMaterial(Material.StaticFriction, Material.DynamicFriction, Material.Restitution);
    physx::PxShape* SphereShape = physx::PxRigidActorExt::createExclusiveShape(*Obj.Body, physx::PxSphereGeometry(Radius), *Obj.Material);
    Obj.MyIndex = ObjectsPool.size() - 1;
    Obj.Body->userData = (void*)(Obj.MyIndex + 1);
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
    Obj.Body->userData = (void*)(Obj.MyIndex + 1);
    Scene->addActor(*Obj.Body);
    return Obj.MyIndex;
}

gdr::gdr_index gdr::physics::NewStaticMesh(physic_material Material, std::vector<vertex> VertexBuffer, std::vector<UINT32> Index, std::string name)
{
    ObjectsPool.emplace_back(new gdr_physics_object()); // TODO - change on smart choosing
    gdr_physics_object& Obj = *ObjectsPool[ObjectsPool.size() - 1];
    Obj.Body = Physics->createRigidStatic(physx::PxTransform(physx::PxIdentity));
    Obj.Material = Physics->createMaterial(Material.StaticFriction, Material.DynamicFriction, Material.Restitution);
    Obj.MyIndex = ObjectsPool.size() - 1;
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
            Shape->userData = (void*)(Obj.MyIndex + 1);
            Shape->release();
        }
    }
    Obj.Name = name;
    Obj.Body->userData = (void*)(Obj.MyIndex + 1);
    Scene->addActor(*Obj.Body);
    return Obj.MyIndex;
}

gdr::gdr_index gdr::physics::NewDynamicMesh(physic_material Material, std::vector<vertex> VertexBuffer, std::vector<UINT32> Index, std::string name)
{
    ObjectsPool.emplace_back(new gdr_physics_object()); // TODO - change on smart choosing
    gdr_physics_object& Obj = *ObjectsPool[ObjectsPool.size() - 1];
    Obj.Body = Physics->createRigidDynamic(physx::PxTransform(physx::PxIdentity));
    Obj.Material = Physics->createMaterial(Material.StaticFriction, Material.DynamicFriction, Material.Restitution);
    Obj.MyIndex = ObjectsPool.size() - 1;
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
        convexDesc.flags = physx::PxConvexFlag::eCOMPUTE_CONVEX | physx::PxConvexFlag::eGPU_COMPATIBLE;
        convexDesc.vertexLimit = 64;

        physx::PxDefaultMemoryOutputStream buf;
        if (!Cooking->cookConvexMesh(convexDesc, buf))
            throw "Cannot ConvexHull";
        physx::PxDefaultMemoryInputData readBuffer(buf.getData(), buf.getSize());
        physx::PxConvexMesh* Model = Physics->createConvexMesh(readBuffer);

        physx::PxShape* Shape = Physics->createShape(physx::PxConvexMeshGeometry(Model), *Obj.Material);
        Obj.Body->attachShape(*Shape);
        Shape->userData = (void*)(Obj.MyIndex + 1);
        Shape->release();
    }
    Obj.Name = name;
    Obj.Body->userData = (void*)(Obj.MyIndex + 1);
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

std::vector<gdr::gdr_index> gdr::physics::NewStaticMeshAssimp(physic_material Material, std::string Filename)
{
  // Create an instance of the Importer class
  Assimp::Importer importer;

  const aiScene* scene = importer.ReadFile(Filename, aiProcess_PreTransformVertices);

  std::vector<gdr::gdr_index> result;

  for (int i = 0; i < scene->mNumMeshes; i++)
  {
    std::vector<vertex> vertices;
    std::vector<UINT32> indices;
    aiMesh *mesh = scene->mMeshes[i];

    for (int j = 0; j < (int)mesh->mNumFaces; j++)
    {
      if (mesh->mFaces[j].mNumIndices != 3)
      {
        continue;
      }
      indices.push_back(mesh->mFaces[j].mIndices[0]);
      indices.push_back(mesh->mFaces[j].mIndices[1]);
      indices.push_back(mesh->mFaces[j].mIndices[2]);
    }

    for (int j = 0; j < (int)mesh->mNumVertices; j++)
    {
      vertex V;
      V.Pos = mth::vec3f({ mesh->mVertices[j].x, mesh->mVertices[j].y, mesh->mVertices[j].z });

      vertices.push_back(V);
    }

    result.push_back(NewStaticMesh(Material, vertices, indices, mesh->mName.C_Str()));
  }

  importer.FreeScene();
  return result;
}
std::vector<gdr::gdr_index> gdr::physics::NewDynamicMeshAssimp(physic_material Material, std::string Filename)
{
  // Create an instance of the Importer class
  Assimp::Importer importer;

  const aiScene* scene = importer.ReadFile(Filename, aiProcess_PreTransformVertices);

  std::vector<gdr::gdr_index> result;

  for (int i = 0; i < scene->mNumMeshes; i++)
  {
    std::vector<vertex> vertices;
    std::vector<UINT32> indices;
    aiMesh* mesh = scene->mMeshes[i];

    for (int j = 0; j < (int)mesh->mNumFaces; j++)
    {
      if (mesh->mFaces[j].mNumIndices != 3)
      {
        continue;
      }
      indices.push_back(mesh->mFaces[j].mIndices[0]);
      indices.push_back(mesh->mFaces[j].mIndices[1]);
      indices.push_back(mesh->mFaces[j].mIndices[2]);
    }

    mth::vec3f Center;

    for (int j = 0; j < (int)mesh->mNumVertices; j++)
    {
      vertex V;
      V.Pos = mth::vec3f({ mesh->mVertices[j].x, mesh->mVertices[j].y, mesh->mVertices[j].z });
      Center += V.Pos;
      vertices.push_back(V);
    }

    Center /= mesh->mNumVertices;

    for (int j = 0; j < (int)mesh->mNumVertices; j++)
    {
      vertices[j].Pos -= Center;
    }

    result.push_back(NewDynamicMesh(Material, vertices, indices, mesh->mName.C_Str()));
    ObjectsPool[result[result.size() - 1]]->SetPos(Center);
  }

  importer.FreeScene();
  return result;
}