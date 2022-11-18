#include "p_header.h"

#include "unit_physic_spheres.h"

void unit_phys_spheres::Initialize(void)
{
}

void unit_phys_spheres::Response(void)
{
    if (Engine->KeysClick['N'])
    {
        SpheresRender.push_back(Engine->ObjectSystem->CreateObjectFromFile("bin/models/pbr_sphere/pbr_sphere.glb"));
        SpheresPhysic.push_back(Engine->NewDynamicSphere(gdr::physic_material(), 1, "sphere"));
        Engine->GetPhysObject(SpheresPhysic[SpheresPhysic.size() - 1]).SetPos(Engine->PlayerCamera.GetPos() + Engine->PlayerCamera.GetDir());
        Engine->GetPhysObject(SpheresPhysic[SpheresPhysic.size() - 1]).AddVelocity(Engine->PlayerCamera.GetDir() * 50);
    }

    // update each sphere according to its transform
    for (int i = 0; i < SpheresRender.size(); i++)
    {
        Engine->ObjectSystem->NodesPool[SpheresRender[i]].GetTransformEditable() = Engine->GetPhysObject(SpheresPhysic[i]).GetTransform();
    }
}