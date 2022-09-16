#pragma once
#include "def.h"

// project namespace
namespace gdr
{
  class timer_support
  {
  protected:
    /* Timer data */
    float
      GlobalTime, GlobalDeltaTime, /* Global time and interframe interval */
      Time, DeltaTime,             /* Time with pause and interframe interval */
      FPS;                         /* Frames per seond value */
    BOOL
      IsPause,                     /* Pause flag */
      IsSleep;                     /* Sleep flag */
  private:
    /* Timer local data */
    UINT64
      StartTime,  /* Start program time */
      OldTime,    /* Time from program start to previous frame */
      OldTimeFPS, /* Old time FPS measurement */
      PauseTime,  /* Time during pause period */
      TimePerSec, /* Timer resolution */
      FrameCounter; /* Frames counter */
    float
      SleepTime;  /* Time during pause */
  
  public:
    /* Default constructor method
     *   ARGUMENTS: none.
     *   RETURNS  :
     *     - new timer;
     */
    timer_support(VOID) : PauseTime(0), FrameCounter(0), IsPause(FALSE), IsSleep(FALSE), FPS(50), SleepTime(0)
    {
      LARGE_INTEGER t;
  
      /* Timer initialization */
      QueryPerformanceFrequency(&t);
      TimePerSec = t.QuadPart;
      QueryPerformanceCounter(&t);
      StartTime = OldTime = OldTimeFPS = t.QuadPart;
    }
  
    /* Getting timer freqensy method
     *   ARGUMENTS: none.
     *   RETURNS  :
     *     - time per sec
     *         UINT64 TimePerSec;
     */
    UINT64 GetTimePerSec(VOID)
    {
      return TimePerSec;
    }
  
    /* Getting time method
     *   ARGUMENTS: none.
     *   RETURNS  :
     *     - time
     *         DBL Time;
     */
    float GetTime(VOID)
    {
      return Time;
    }
  
    /* Getting time method
     *   ARGUMENTS: none.
     *   RETURNS  :
     *     - time
     *         DBL Time;
     */
    float GetFPS(VOID)
    {
      return FPS;
    }
  
    /* Getting global time method
     *   ARGUMENTS: none.
     *   RETURNS  :
     *     - global time
     *         DBL GlobalTime;
     */
    float GetGlobalTime(VOID)
    {
      return GlobalTime;
    }
  
    /* Getting delta time method
     *   ARGUMENTS: none.
     *   RETURNS  :
     *     - delta time
     *         DBL DeltaTime;
     */
    float GetDeltaTime(VOID)
    {
      return DeltaTime;
    }
  
    /* Getting global delta time method
     *   ARGUMENTS: none.
     *   RETURNS  :
     *     - time
     *         DBL GlobalDeltaTime;
     */
    float GetGlobalDeltaTime(VOID)
    {
      return GlobalDeltaTime;
    }
  
    /* Increase FPS Counter method
     *   ARGUMANTS: none.
     *   RETURNS  : none.
     */
    VOID IncreaseFrameCounter(VOID)
    {
      FrameCounter++;                    // increment frame counter (for FPS)
    } /* End of 'IncreaseFrameCounter' method */
  
    /* Timer response method
     *   ARGUMANTS: none.
     *   RETURNS  : none.
     */
    VOID Response(VOID)
    {
      LARGE_INTEGER t;
  
      /*** Handle timer ***/
      QueryPerformanceCounter(&t);           // obtain current timer value
      /* Global time */
      GlobalTime = (float)(t.QuadPart - StartTime) / TimePerSec;
      GlobalDeltaTime = (float)(t.QuadPart - OldTime) / TimePerSec;
      /* Time with pause */
      if (IsPause || IsSleep)
      {
        PauseTime += t.QuadPart - OldTime;
        DeltaTime = 0;
      }
      else
      {
        Time = (float)(t.QuadPart - PauseTime - StartTime) / TimePerSec;
        DeltaTime = GlobalDeltaTime;
      }
      /* Sleep */
      if (SleepTime)
      {
        SleepTime -= GlobalDeltaTime;
        if (SleepTime <= 0)
        {
          SleepTime = 0;
          IsSleep = FALSE;
        }
      }
      /* FPS */
      if (t.QuadPart - OldTimeFPS > TimePerSec)
      {
        FPS = (float)FrameCounter * TimePerSec / (t.QuadPart - OldTimeFPS);
        OldTimeFPS = t.QuadPart;
        FrameCounter = 0;
        OutputDebugString((std::to_wstring(FPS) + L"\n").c_str());
      }
      OldTime = t.QuadPart;
    }
  
    /* Set pause method
     *   ARGUMENTS: none.
     *   RETURNS  : none.
     */
    VOID SetPause(BOOL Type)
    {
      IsPause = Type;
    }
  
    /* Toggle pause method
     *   ARGUMENTS: none.
     *   RETURNS  : none.
     */
    VOID TogglePause(void)
    {
      IsPause = !IsPause;
    }
  
    BOOL GetPause(void)
    {
      return IsPause;
    }
  
    /* Set sleep time method
     *   ARGUMENTS:
     *     - time to sleep(seconds)
     *       INT S;
     *   RETURNS: none.
     */
    VOID Sleep(float S)
    {
      if (S == 0)
        return;
      SleepTime += S;
      IsSleep = TRUE;
    }
  }; /* End of 'timer' class */
}