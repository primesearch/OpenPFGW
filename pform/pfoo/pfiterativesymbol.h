#ifndef PFITERATIVESYMBOL_H
#define PFITERATIVESYMBOL_H

#include "pffunctionsymbol.h"

class PFIterativeSymbol : public PFFunctionSymbol
{
// the context variable allows us to get the prompt handler
protected:
	PFSymbolTable *m_pContext;
	
protected:
	uint32_t	m_dwStepGranularity;
   uint32_t m_dwStepsDone;
   uint32_t m_dwStepsTotal;

	clock_t m_tStartTime;
	clock_t m_tLastTimePrompt;
	clock_t m_tLastTimeSerialized;
	
	PFBoolean m_bStopOverride;
   uint32_t m_dwTotalSteps;
public:
    // overall result
    int testResult;
protected:
	PFIterativeSymbol(const PFString &sName);
	PFIterativeSymbol(const PFIterativeSymbol &s);
	PFIterativeSymbol &operator=(const PFIterativeSymbol &s);

	static int lastLineLen;

	
	// note that CallFunction returns b_true if the operation was completed
	
	virtual PFBoolean OnExecute(PFSymbolTable *pContext)=0;
	// always called whenever the function is restarted

	// completed is called on a successful completion
	// cleanup is called in all cases
	virtual PFBoolean OnCompleted(PFSymbolTable *pContext)=0;
	virtual PFBoolean OnCleanup(PFSymbolTable *pContext)=0;
	
	virtual PFBoolean OnInitialize()=0;
	// initial setup only on first iteration
	
	virtual void OnPrompt();
	// can be overridden to recompute time remaining, etc
	
	void ShowPrompt();
	// writes the prompt to the current console.
	
	void UpdatePromptTime(DWORD dwElapsed);
	// adjust timing to reflect settings
	
	PFBoolean ExceededSaveTime(DWORD dwElapsed);
	// do we need to do a save anyway?
	
	PFBoolean IsRunning();
	// checks the current console for exit
	
	virtual PFBoolean Iterate()=0;
	// returns true if the test is complete

public:
	PFBoolean CallFunction(PFSymbolTable *pContext);
};
#endif
