/*
 * This is just a wrapper for PFGW
 * Currently it initializes clocks_per_sec to correct values
 */

#include "primeformpch.h"

#include <time.h>
#if defined (_MSC_VER)
#include <float.h>
#endif

#include "../../pform/pfio/pfini.h"
#include "pfgw_globals.h"

//#define COMPUTE_ALLOCATION_COUNTS
#include "gmp_mem.cxx"

#ifdef _MSC_VER
#include <crtdbg.h>
#endif

extern unsigned long clocks_per_sec;


int main(int argc, char *argv[])
{
	clocks_per_sec=CLOCKS_PER_SEC;

	// Make sure that VC uses 64 bit FPU instructions for high level FPU code
#if defined (_MSC_VER)
	_control87(_PC_64, _MCW_PC);	// 64 bits precision (instead of 53 bit default precision)
	_control87(_RC_NEAR, _MCW_RC);	// make SURE that we round numbers to the nearest, and not floor or ceil
#endif

	// Simple memory debugging (Only availible under VC, but the testing will allow ALL OS's to benefit.
#if defined (_MSC_VER) && defined (_DEBUG)
	_CrtMemState mem_dbg1, mem_dbg2, mem_dbg3;
// Send all reports to STDOUT
   _CrtSetReportMode( _CRT_WARN, _CRTDBG_MODE_FILE );
   _CrtSetReportFile( _CRT_WARN, _CRTDBG_FILE_STDOUT );
   _CrtSetReportMode( _CRT_ERROR, _CRTDBG_MODE_FILE );
   _CrtSetReportFile( _CRT_ERROR, _CRTDBG_FILE_STDOUT );
   _CrtSetReportMode( _CRT_ASSERT, _CRTDBG_MODE_FILE );
   _CrtSetReportFile( _CRT_ASSERT, _CRTDBG_FILE_STDOUT );
// Store a memory checkpoint in the s1 memory-state structure
   _CrtMemCheckpoint( &mem_dbg1 );

   // Get the current state of the flag
   int tmpFlag = _CrtSetDbgFlag( _CRTDBG_REPORT_FLAG );
   // Turn Off (AND) - DO NOT Enable debug heap allocations and use of memory block type identifiers such as _CLIENT_BLOCK
   //tmpFlag &= ~_CRTDBG_ALLOC_MEM_DF;
   tmpFlag |= _CRTDBG_ALLOC_MEM_DF;

   // Turn Off (AND) - prevent _CrtCheckMemory from being called at every allocation request.  _CurCheckMemory must be called explicitly
   tmpFlag &= ~_CRTDBG_CHECK_ALWAYS_DF;
   //tmpFlag |= _CRTDBG_CHECK_ALWAYS_DF;

   // Turn Off (AND) - Do NOT include __CRT_BLOCK types in leak detection and memory state differences
   tmpFlag &= ~_CRTDBG_CHECK_CRT_DF;
   //tmpFlag |= _CRTDBG_CHECK_CRT_DF;

   // Turn Off (AND) - DO NOT Keep freed memory blocks in the heap’s linked list and mark them as freed
   tmpFlag &= ~_CRTDBG_DELAY_FREE_MEM_DF;

   // Turn Off (AND) - Do NOT perform leak check at end of program run.
   //tmpFlag &= ~_CRTDBG_LEAK_CHECK_DF;
   tmpFlag |= _CRTDBG_LEAK_CHECK_DF;

   // Set the new state for the flag
   _CrtSetDbgFlag( tmpFlag );

#endif

	memAlloc();

	// Setup the global ini object to the PFGW.INI file.  NOTE that any program written which will be
	// calling pfgw_main() will need to open the "correct" ini file.  PFGW.EXE opens PFGW.ini.  WinPFGW.exe
	// may open something different and foobars_speed_siever.exe may open up something altogether different.

	// Create a console output object.
	pOutputObj = new PFConsoleOutput;

	g_pIni = new PFIni("pfgw.ini");
	g_pIni->SetCurrentSection("PFGW");

	int ret = pfgw_main(argc, argv);

	delete pOutputObj;
	delete g_pIni;

	memFree();

#if defined (_MSC_VER) && defined (_DEBUG)
	_CrtMemCheckpoint( &mem_dbg2 );
   if ( _CrtMemDifference( &mem_dbg3, &mem_dbg1, &mem_dbg2 ) )
   {
	   fprintf (stderr, "\nDump the changes that occurred between two memory checkpoints\n");
	   _CrtMemDumpStatistics( &mem_dbg3 );
//	   _CrtDumpMemoryLeaks( );
   }
   else
	   fprintf (stderr, "\nNo memory leaks found\n");
#endif

	return ret;
}

