#pragma once
#include "unit_base.h"

class unit_phys_spheres : public gdr::unit_base
{
private:
    std::vector<gdr::gdr_index> SpheresRender;
    std::vector<gdr::gdr_index> SpheresPhysic;
public:
    void Initialize(void);

    void Response(void);

    std::string GetName(void)
    {
        return "unit_phys_spheres";
    }

    ~unit_phys_spheres(void)
    {
    }
};