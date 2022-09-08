#include "p_header.h"

// Default constructor
gdr::render::render() : IsInited(true)
{
}

/* Initialize function
 * ARGUMENTS:
 *      - Engine to init
 *          (engine *) Eng
 * RETURNS: true if success, false otherwise
 */
bool gdr::render::Init(engine* Eng)
{
  Engine = Eng;

  IsInited = true;
  
  IsInited = Device.Init(gdr::device_create_params());

  return IsInited;
}

/* Initialize function
 * ARGUMENTS:
 *      - Engine to init
 *          (engine *) Eng
 * RETURNS: true if success, false otherwise
 */
void gdr::render::Term(void)
{
  if (!IsInited)
    return;

  Device.Term();
}

// Default destructor
gdr::render::~render()
{
  Term();
}