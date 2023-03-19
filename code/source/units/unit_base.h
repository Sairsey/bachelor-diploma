#pragma once

#include "def.h"


/* Project namespace */
namespace gdr
{
  class engine;
  /* Unit class declaration */
  class unit_base
  {
    public:
      engine* Engine; // pointer to Engine
      unit_base *ParentUnit; // pointer on parent unit
      std::vector<unit_base *> ChildUnits; // Child units in hierarchy
      // Variables of this unit
      std::unordered_map<std::string, gdr_index> IndicesVars;
      std::unordered_map<std::string, float> FloatVars;
      std::unordered_map<std::string, float2> Float2Vars;
      std::unordered_map<std::string, float3> Float3Vars;
      std::unordered_map<std::string, float4> Float4Vars;
      std::unordered_map<std::string, mth::matr4f> MatrVars;
      std::unordered_map<std::string, std::string> StringVars;

      /* default constructor */
      unit_base(void)
      {
      }

      // Return name of this unit
      virtual std::string GetName(void)
      {
        return "unit_base";
      }

      /* Initialization function.
       * ARGUMENTS: None.
       * RETURNS: None.
       */
      virtual void Initialize(void)
      {
      }

      /* Response function which will be called on every frame.
       * ARGUMENTS: None.
       * RETURNS: None.
       */
      virtual void Response(void)
      {
      }

      /* Response function which will be called on every PHYSICS_TICK.
       * ARGUMENTS: None.
       * RETURNS: None.
       */
      virtual void ResponsePhys(void)
      {
      }

      /* Destructor */
      virtual ~unit_base(void)
      {
      }
  };
}