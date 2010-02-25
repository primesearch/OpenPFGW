#if defined(_MSC_VER) && defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#define DEBUG_NEW new(_NORMAL_BLOCK, __FILE__, __LINE__)
#define new DEBUG_NEW
#endif

#include "pfoopch.h"
#include "pfiterativesymbol.h"
#include "pfintegersymbol.h"
#include "pfgw_globals.h"

char g_cpTestString[70];
bool g_bHaveFatalError;
PFString g_sTestMode;            // This will hold things like "PRP: ", "N+1: ", "F: ", "GF(b,3): ", ...

int PFIterativeSymbol::lastLineLen;

PFIterativeSymbol::PFIterativeSymbol(const PFString &sName)
   :  PFFunctionSymbol(sName), m_pContext(NULL), m_dwStepGranularity(4096), m_dwStepsDone(0), m_dwStepsTotal(0),
      m_tStartTime(0), m_tLastTimePrompt(0), m_tLastTimeSerialized(0),
      m_bStopOverride(PFBoolean::b_false), m_dwTotalSteps(0), testResult(0)
{
}

PFIterativeSymbol::PFIterativeSymbol(const PFIterativeSymbol &s)
   :  PFFunctionSymbol(s.GetKey()), m_pContext(NULL), m_dwStepGranularity(4096), m_dwStepsDone(0), m_dwStepsTotal(0),
      m_tStartTime(0), m_tLastTimePrompt(0), m_tLastTimeSerialized(0),
      m_bStopOverride(PFBoolean::b_false), m_dwTotalSteps(0), testResult(0)
{
}

PFIterativeSymbol &PFIterativeSymbol::operator=(const PFIterativeSymbol &/*s*/)
{
   m_pContext=NULL;
   return *this;
}

extern int g_nIterationCnt;      // located in pfgw_main.cpp
extern bool volatile g_bExitNow;

PFBoolean PFIterativeSymbol::CallFunction(PFSymbolTable *pContext)
{
   m_pContext=pContext;
   PFBoolean bRetval=OnExecute(pContext);
   DWORD dwFrom=0;

   g_bHaveFatalError = false;

   if(dwFrom==0)
   {
      m_dwStepGranularity=4096;
      m_dwStepsDone=0;
      m_tStartTime=clock();
      bRetval=OnInitialize();
   }

   if(bRetval)
   {
      if (g_nIterationCnt)
      {
         m_tLastTimePrompt=m_tLastTimeSerialized=clock();
         OnPrompt();
         ShowPrompt();
      }

      bool bFirst=true;
      while(m_bStopOverride||(m_dwStepsDone<m_dwStepsTotal))
      {
         bRetval=Iterate();
         m_dwStepsDone++;
         if (g_bExitNow)
         {
            OnCleanup(pContext);
            return PFBoolean::b_false;
         }
         if(!bRetval)
         {
            if (g_nIterationCnt)
            {
               if(g_nIterationCnt && (((m_dwStepsDone%g_nIterationCnt)==0) || bFirst || (m_dwStepsDone==m_dwStepsTotal)))
               {
                  bFirst=false;
                  // 120 bytes will not overflow, since we "force" the max size below
                  char Buf[120];
                  if (ErrorCheck(m_dwStepsDone, m_dwStepsTotal))
                  {
                     if (*g_cpTestString)
                        sprintf(Buf,"%s%.50s %lu/%lu mro=%0.10g\r",LPCTSTR(g_sTestMode),g_cpTestString, m_dwStepsDone,m_dwStepsTotal, g_dMaxError);
                     else
                        sprintf(Buf,"%s%lu/%lu mro=%0.10g\r",LPCTSTR(g_sTestMode),m_dwStepsDone,m_dwStepsTotal, g_dMaxError);
                  }
                  else
                  {
                     if (*g_cpTestString)
                        sprintf(Buf,"%s%.50s %lu/%lu\r",LPCTSTR(g_sTestMode),g_cpTestString, m_dwStepsDone,m_dwStepsTotal);
                     else
                        sprintf(Buf,"%s%lu/%lu\r",LPCTSTR(g_sTestMode),m_dwStepsDone,m_dwStepsTotal);
                  }
                  int thisLineLen = strlen(Buf);
                  if (lastLineLen > thisLineLen)
                     // When mixing stdio, stderr and redirection with a \r stderr output,
                     // then the line must "erase" itself, IF it ever shrinks.
                     PFPrintfClearCurLine(lastLineLen);
                  lastLineLen = thisLineLen;
                  PFPrintfStderr("%s", Buf);
               }

               if(((m_dwStepsDone%m_dwStepGranularity)==0)||(!IsRunning()))
               {
                  clock_t now=clock();
                  DWORD dwElapsed;

                  if(IsRunning())
                  {
                     dwElapsed=(now-m_tLastTimePrompt);
                     OnPrompt();
                     ShowPrompt();
                     m_tLastTimePrompt=now;
                     UpdatePromptTime(dwElapsed);
                  }

                  dwElapsed=now-m_tLastTimeSerialized;

                  if(ExceededSaveTime(dwElapsed)||(!IsRunning()))
                  {
                     // perform serialization here
                  }
                  bRetval=IsRunning();
               }
            }
         }
         else
         {
            if (g_bHaveFatalError)
            {
               PFPrintfClearCurLine(lastLineLen);
               lastLineLen = 0;
               OnCompleted(pContext);
               pContext->AddSymbol(new PFIntegerSymbol("_result",new Integer(0)));
               OnCleanup(pContext);
               return PFBoolean::b_false;
            }
            break;   //we are finished
         }
      }  // end while loop
      if (g_nIterationCnt)
         PFPrintfClearCurLine(lastLineLen);
      lastLineLen = 0;
   }  // endif

   bRetval=IsRunning();

   if(bRetval)
   {
      bRetval=OnCompleted(pContext);
      pContext->AddSymbol(new PFIntegerSymbol("_result",new Integer(testResult)));
   }
   OnCleanup(pContext);

   return bRetval;
   // bRetval is true if the test completed successfully. Non-completion
   // could mean an error or it could mean a quit.
}

void PFIterativeSymbol::OnPrompt()
{
   // do prompt update
}

void PFIterativeSymbol::ShowPrompt()
{
   // display the prompt
}

void PFIterativeSymbol::UpdatePromptTime(DWORD /*dwElapsed*/)
{
   // use the console settings to adjust your tick timer
}

PFBoolean PFIterativeSymbol::ExceededSaveTime(DWORD /*dwElapsed*/)
{
   // is it time to save?
   return PFBoolean::b_false;
}

PFBoolean PFIterativeSymbol::IsRunning()
{
   // ask the console if it is still running
   return PFBoolean::b_true;
}


