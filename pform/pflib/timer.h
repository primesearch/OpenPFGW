#ifndef GOT_TIMER_H
#define GOT_TIMER_H
#include <time.h>
#include <sys/timeb.h>
#if defined (_MSC_VER) || defined (__MINGW32__)
#include <windows.h>
typedef LARGE_INTEGER hr_timer;
#define HRZERO(X)				(X).HighPart = (X).LowPart = 0
#define HRSETCURRENT(X)			QueryPerformanceCounter (&(X));
#define HRGETTICKS(X)			((double)(X).HighPart*4294967296.0+(double)(X).LowPart)
#define HRGETTICKS_PER_SEC(X)	{LARGE_INTEGER large;														\
								   if (QueryPerformanceFrequency (&large))									\
								      (X) = (double)large.HighPart*4294967296.0 +	(double)large.LowPart;	\
								   else																		\
								      (X) = 0.0;															\
								}
#else
#include <sys/time.h>
typedef struct timeval hr_timer;
#define HRZERO(X)				(X).tv_sec = (X).tv_usec = 0
#define HRSETCURRENT(X)			{struct timezone tz; gettimeofday (&(X), &tz);}
#define HRGETTICKS(X)			((double)(X).tv_sec*1000000.0+(double)(X).tv_usec)
#define HRGETTICKS_PER_SEC(X)	(X) = 1000000.0
#endif

class CTimer
{
public:
	CTimer ();
	void Start ();					// Start the timer
	void Stop ();					// Stop the timer
	double GetSecs ();				// If timer is running returns elapsed;
									// if stopped returns timed interval;
									// if not started returns 0.0.
private:
	static double sm_HRTicksPerSec;	// HR Ticks per second
	static bool sm_fGotHRTicksPerSec;// Set if we have got the above
	static double sm_hrPrecision;
	static double sm_cPrecision;
	bool m_fRunning;				// true if we are running
	clock_t m_cStartTime;
	clock_t m_cEndTime;
	hr_timer m_hrStartTime;
	hr_timer m_hrEndTime;
};
#endif
