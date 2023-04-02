#pragma once
#include "unit_base.h"
#include "editor/unit_scripted.h"
#include <fstream>
#include <sstream>
#include "json.hpp"

/* Unit class declaration */
class unit_startup_launch : public gdr::unit_base
{
public:
  /* default constructor */
  unit_startup_launch(void)
  {
  }

  // Return name of this unit
  virtual std::string GetName(void)
  {
    return "unit_startup_launch";
  }

  // trim from start (in place)
  static inline void ltrim(std::string& s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
      return !std::isspace(ch);
      }));
  }

  // trim from end (in place)
  static inline void rtrim(std::string& s) {
    s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
      return !std::isspace(ch);
      }).base(), s.end());
  }

  // trim from both ends (in place)
  static inline void trim(std::string& s) {
    rtrim(s);
    ltrim(s);
  }


  /* Initialization function.
   * ARGUMENTS: None.
   * RETURNS: None.
   */
  void Initialize(void) override
  {
    std::ifstream configFile("startup.ini");

    if (!configFile.is_open())
      return;

    std::string line;
    while (std::getline(configFile, line))
    {
      trim(line);
      if (line[0] == '#')
        continue;
      else
        Engine->UnitsManager->Add(new unit_scripted(line), Me);
    }
  }

  /* Response function which will be called on every frame.
   * ARGUMENTS: None.
   * RETURNS: None.
   */
  void Response(void) override
  {
  }

  /* Response function which will be called on every PHYSICS_TICK.
   * ARGUMENTS: None.
   * RETURNS: None.
   */
  void ResponsePhys(void) override
  {
  }

  /* Destructor */
  ~unit_startup_launch(void) override
  {
  }
};
