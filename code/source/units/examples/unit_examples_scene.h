#pragma once
#include "../unit_base.h"
#include "unit_triangle.h"
#include "unit_light_example.h"
#include "unit_model_loading.h"
#include "unit_frustum_cull.h"
#include "unit_occlusion_cull.h"
#include "unit_runtime_add_remove.h"
#include "unit_pbr_metalness.h"
#include "unit_pbr_specular.h"
#include "unit_normal_map.h"
#include "unit_load_any.h"
#include "unit_transparent.h"
#include "unit_model_splitted.h"
#include "unit_specialist.h"
#include "unit_pit.h"
#include "unit_choose.h"

#include "../cameras/unit_flying_camera.h"
#include "../enviroment/unit_alex_apt.h"

enum struct EXAMPLES_ENUM
{
  None = 0,
  Triangle,
  Light,
  ModelLoad,
  FrustumCull,
  OcclusionCull,
  RuntimeAddRemove,
  PbrMetalness,
  PbrSpecular,
  NormalMap,
  LoadAny,
  Transparent,
  ModelSplitted,
  Specialist,
  Pit,

  COUNT
};

/* Unit class declaration */
class unit_examples_scene : public gdr::unit_base
{
private:
  EXAMPLES_ENUM Inited = EXAMPLES_ENUM::None;
  EXAMPLES_ENUM Asked;
  gdr_index shown_example = NONE_INDEX;
public:
  /* default constructor */
  unit_examples_scene(EXAMPLES_ENUM ask = EXAMPLES_ENUM::Triangle) : Asked(ask)
  {
  }

  // Return name of this unit
  virtual std::string GetName(void)
  {
    return "unit_examples_scene";
  }

  /* Initialization function.
   * ARGUMENTS: None.
   * RETURNS: None.
   */
  void Initialize(void) override
  {
    Engine->UnitsManager->Add(new unit_flying_camera(), Me);
    Engine->UnitsManager->Add(new unit_alex_apt(), Me);
  }

  /* Response function which will be called on every frame.
   * ARGUMENTS: None.
   * RETURNS: None.
   */
  void Response(void) override
  {
    if (Inited != Asked)
    {
      if (shown_example != NONE_INDEX)
        Engine->UnitsManager->Remove(shown_example);
       
      switch (Asked)
      {
      case EXAMPLES_ENUM::None:
        break;
      case EXAMPLES_ENUM::Triangle:
        shown_example = Engine->UnitsManager->Add(new unit_triangle(), Me);
        break;
      case EXAMPLES_ENUM::Light:
        shown_example = Engine->UnitsManager->Add(new unit_light_example(), Me);
        break;
      case EXAMPLES_ENUM::ModelLoad:
        shown_example = Engine->UnitsManager->Add(new unit_model_loading(), Me);
        break;
      case EXAMPLES_ENUM::FrustumCull:
        shown_example = Engine->UnitsManager->Add(new unit_frustum_cull(), Me);
        break;
      case EXAMPLES_ENUM::OcclusionCull:
        shown_example = Engine->UnitsManager->Add(new unit_occlusion_cull(), Me);
        break;
      case EXAMPLES_ENUM::RuntimeAddRemove:
        shown_example = Engine->UnitsManager->Add(new unit_runtime_add_remove(), Me);
        break;
      case EXAMPLES_ENUM::PbrMetalness:
        shown_example = Engine->UnitsManager->Add(new unit_pbr_metalness(), Me);
        break;
      case EXAMPLES_ENUM::PbrSpecular:
        shown_example = Engine->UnitsManager->Add(new unit_pbr_specular(), Me);
        break;
      case EXAMPLES_ENUM::NormalMap:
        shown_example = Engine->UnitsManager->Add(new unit_normal_map(), Me);
        break;
      case EXAMPLES_ENUM::LoadAny:
        shown_example = Engine->UnitsManager->Add(new unit_load_any(), Me);
        break;
      case EXAMPLES_ENUM::Transparent:
        shown_example = Engine->UnitsManager->Add(new unit_transparent(), Me);
        break;
      case EXAMPLES_ENUM::ModelSplitted:
        shown_example = Engine->UnitsManager->Add(new unit_model_splitted(), Me);
        break;
      case EXAMPLES_ENUM::Specialist:
        shown_example = Engine->UnitsManager->Add(new unit_specialist(), Me);
        break;
      case EXAMPLES_ENUM::Pit:
        shown_example = Engine->UnitsManager->Add(new unit_pit(), Me);
        break;
      case EXAMPLES_ENUM::COUNT:
        break;
      default:
        break;
      }
      Inited = Asked;
    }
      
  }

  /* Destructor */
  ~unit_examples_scene(void) override
  {
  }
};
