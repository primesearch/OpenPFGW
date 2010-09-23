
#include "config.h"
#include "timer.h"
#include <stdio.h>

double CTimer::sm_HRTicksPerSec = 0.0;
bool CTimer::sm_fGotHRTicksPerSec = false;
double CTimer::sm_hrPrecision = 0.0;
double CTimer::sm_cPrecision = 0.0;

CTimer::CTimer ()
: m_fRunning(false), m_cStartTime(0), m_cEndTime(0), m_hrStartTime(), m_hrEndTime()

{
   HRZERO (m_hrStartTime);
   HRZERO (m_hrEndTime);

   if (!sm_fGotHRTicksPerSec)
   {
      sm_fGotHRTicksPerSec = true;
        // What's the lowest digit set non-zero in a clock() call
      // That's a fair indication what the precision is likely to be.
      // Note - this isn't actually used
      clock_t heuristicTimeTest=clock();
      if(heuristicTimeTest%10) sm_cPrecision = 1.0/CLOCKS_PER_SEC;
      else if(heuristicTimeTest%100) sm_cPrecision = 10.0/CLOCKS_PER_SEC;
      else if(heuristicTimeTest%1000) sm_cPrecision = 100.0/CLOCKS_PER_SEC;
      else if(heuristicTimeTest%10000) sm_cPrecision = 1000.0/CLOCKS_PER_SEC;
      else sm_cPrecision = 10000.0/CLOCKS_PER_SEC;

        // Find the claimed resolution of the high res timer
      // Then find the most likely real rate by waiting for it to change.
      // Note - I've frequently seen missed beats, and therefore a
      // 0.000001 reality gets reported as a 0.000002.
      // Note - this also isn't actually used, all that matters is
      // whether HRTicksPerSec has a non-zero value or not.
      HRGETTICKS_PER_SEC (sm_HRTicksPerSec);

      if (sm_HRTicksPerSec != 0.0)
      {
         hr_timer start, end;
         HRSETCURRENT (start);

         do
         {
            HRSETCURRENT (end);
         }  while (HRGETTICKS (end) == HRGETTICKS (start));

         sm_hrPrecision = (HRGETTICKS (end)-HRGETTICKS (start))/sm_HRTicksPerSec;
      }
   }
   m_fRunning = false;
}


void CTimer::Start ()
{
   if (sm_HRTicksPerSec != 0.0) { HRSETCURRENT (m_hrStartTime); }
   else { m_cStartTime = clock (); }
   m_fRunning = true;
}

void CTimer::Stop ()
{
   if (sm_HRTicksPerSec != 0.0) { HRSETCURRENT (m_hrEndTime); }
   else { m_cEndTime = clock (); }
   m_fRunning = false;
}

double CTimer::GetSecs ()
{
   if (m_fRunning)
   {
      if (sm_HRTicksPerSec != 0.0) { HRSETCURRENT(m_hrEndTime); }
      else { m_cEndTime = clock (); }
   }

   double retval;

   if (sm_HRTicksPerSec == 0.0)
   {
      // This is process time
      retval = (m_cEndTime-m_cStartTime)*1.0/CLOCKS_PER_SEC;
   }
   else
   {
      // This is wall-clock time
      retval = (HRGETTICKS (m_hrEndTime) - HRGETTICKS (m_hrStartTime))/sm_HRTicksPerSec;
   }

   return retval;
}
