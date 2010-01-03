#include <string.h>
#include <ctype.h>

#include "pfiopch.h"
#include "pfscriptfile.h"

extern bool volatile g_bExitNow;

Integer *ex_evaluate(PFSymbolTable *pContext,const PFString &e,int m);

PFScriptFile::PFScriptFile(const char* FileName) 
	: PFSimpleFile(FileName), script(0)
{
	m_pTable = new PFSymbolTable();
	PFFunctionSymbol::LoadExprFunctions(m_pTable);
	m_pTable->AddSymbol(new F_Factor);

	PFIntegerSymbol *pNew;

	Integer *iFF=new Integer(1);
	pNew=new PFIntegerSymbol("FACTORFOUND",iFF);
	m_pTable->AddSymbol(pNew);

	Integer *iMaxF=new Integer(0);
	pNew=new PFIntegerSymbol("MAXF",iMaxF);
	m_pTable->AddSymbol(pNew);

	Integer *iMinF=new Integer(0);
	pNew=new PFIntegerSymbol("MINF",iMinF);
	m_pTable->AddSymbol(pNew);

	PFString s;
	PFStringSymbol *pNewStr=new PFStringSymbol("MODF",s);
	m_pTable->AddSymbol(pNewStr);

	Integer *iVal=new Integer(0);
	pNew=new PFIntegerSymbol("ISPRP",iVal);
	m_pTable->AddSymbol(pNew);

   iVal=new Integer(0);
	pNew=new PFIntegerSymbol("ISPRIME",iVal);
	m_pTable->AddSymbol(pNew);

   Integer *iRetVal=new Integer(0);
	pNew=new PFIntegerSymbol("ERRORLEVEL",iRetVal);
	m_pTable->AddSymbol(pNew);

	// When we start a file, we are NOT in a GoSub at that time.
	m_GosubLevel = 0;

#ifdef _DEBUG
	m_pTable->ListSymbols();
#endif
}	

PFScriptFile::~PFScriptFile() 
{
	int i;
	delete m_pTable;
	if (script) {
		for (i=0;i<m_nNumLines;i++) 
			delete[] script[i];
		free(script);
	}
}

void PFScriptFile::LoadFirstLine() 
{
	m_nNumLines=0;
	char temp[4096];
	char *ptr;
	int i;

		//Get number of lines in file.
	ReadLine(temp,4096);  // Go past header.
	while (!ReadLine(temp,4096))  
		m_nNumLines++;

	if (m_nNumLines==0)
		throw "Error, no lines in script";


	fseek(m_fpInputFile, 0, SEEK_SET);
	ReadLine(temp,4096);  // Go past header.

	script=(char **)malloc(sizeof(char *)*m_nNumLines);
	for (i=0;i<m_nNumLines;i++) {
		ReadLine(temp,4096);
		strtok(temp,"\n\r");
		script[i]=new char[strlen(temp)+1];
		strcpy(script[i],temp);
		if ((ptr=strstr(temp,"LABEL"))!=NULL) {
			ptr+=5;
			while (isspace(*ptr))
				ptr++;
			if (*ptr != '_' && !isalpha(*ptr)) 
				throw "Bad label name";
			int j=1;
			while (isalnum(ptr[j]) || ptr[j] == '_')
				j++;
			ptr[j]=0;
			PFIntSymbol *pLabel=new PFIntSymbol(ptr,i);
			m_pTable->AddSymbol(pLabel);
		}
	}
	
	m_nInstrPtr=0;
	m_bEOF = false;
	if (m_pIni) {// Make sure we don't get weirdness from the 2nd stage constructor.
		m_pIni->SetFileProcessing(false);
		PFString blank;
		m_pIni->SetFileName(&blank);
	}
}

int PFScriptFile::GetNextLine(PFString &sLine, Integer *pInt, bool *pbIntValid, PFSymbolTable *psymRuntime)
{
	bool doTest=false,end=false;
	char *scrPtr;
	char buff[4096];
	char temp[100000];  // Allow for long number expansions
	char t;
	int i;
	if (!pbIntValid || !pInt) {
		m_sCurrentExpression="";
		sLine=m_sCurrentExpression;
		return e_unknown;
	}
	do {
		m_sCurrentExpression="";

		if (m_nInstrPtr>=m_nNumLines || g_bExitNow) {
			end=true;
			break;
		}

		strcpy(temp,script[m_nInstrPtr]);
		scrPtr=temp;
			// Increment the instruction pointer, get it out the way.
		m_nInstrPtr++;

			// Note, line numbers no longer allowed.
		while (isspace(*scrPtr))
			scrPtr++;

			// ok should now be looking at a command, isolate command.
		i=0;
		bool valid_args = false;
		do {
			t=buff[i]=(char)toupper(scrPtr[i]);
			if (!isalpha(t)) {
				if (buff[i])
					valid_args = true;
				buff[i]=0;
			}
			i++;
		} while (isalpha(t));

		if (valid_args)
			scrPtr+=i;
		else
			scrPtr+=i-1;
		while (isspace(*scrPtr))
			scrPtr++;
		
			// command name now in buff, arguments pointed to by scrPtr.
		end=false;
      psymRuntime->RemoveSymbol("_TESTMODE");
      if (!strcmp(buff, "PRIMEP"))
      {
         IPFSymbol *pSymbol=m_pTable->LookupSymbol(scrPtr);

         // If the line is an expression, then we don't need to do any
         // special evaluation
         if (pSymbol != NULL)
         {
            m_pTable->AddSymbol(new PFStringSymbol("_TESTMODE", "P"));
            doTest=true;
            m_sCurrentExpression = pSymbol->GetStringValue();
         }
         else
         {
			   PFOutput::EnableOneLineForceScreenOutput();
			   PFPrintfStderr("Unknown variable name on line %d",m_nInstrPtr+1);
			   end=true;
         }
      }
      else if (!strcmp(buff, "PRIMEM"))
      {
         IPFSymbol *pSymbol=m_pTable->LookupSymbol(scrPtr);

         // If the line is an expression, then we don't need to do any
         // special evaluation
         if (pSymbol != NULL)
         {
            psymRuntime->AddSymbol(new PFStringSymbol("_TESTMODE", "M"));
            doTest=true;
            m_sCurrentExpression = pSymbol->GetStringValue();
         }
         else
         {
			   PFOutput::EnableOneLineForceScreenOutput();
			   PFPrintfStderr("Unknown variable name on line %d",m_nInstrPtr+1);
			   end=true;
         }
      }
      else if (!strcmp(buff, "PRIMEC"))
      {
         IPFSymbol *pSymbol=m_pTable->LookupSymbol(scrPtr);

         // If the line is an expression, then we don't need to do any
         // special evaluation
         if (pSymbol != NULL)
         {
            psymRuntime->AddSymbol(new PFStringSymbol("_TESTMODE", "C"));
            doTest=true;
            m_sCurrentExpression = pSymbol->GetStringValue();
         }
         else
         {
			   PFOutput::EnableOneLineForceScreenOutput();
			   PFPrintfStderr("Unknown variable name on line %d",m_nInstrPtr+1);
			   end=true;
         }
      }
      else if (!strcmp(buff,"PRP")) 
      {
			char *ptr2,*valptr;
			doTest=true;
			bool bText=false;

         IPFSymbol *pSymbol=m_pTable->LookupSymbol(scrPtr);

         // If the line is an expression, then we don't need to do any
         // special evaluation
         if (pSymbol != NULL)
            m_sCurrentExpression = pSymbol->GetStringValue();
         else
         {
			   if ((ptr2=strchr(scrPtr,','))!=NULL) {
				   bText = true;
				   *ptr2=0;
				   ptr2++;
				   while (isspace(*ptr2))
					   ptr2++;
				   pSymbol=m_pTable->LookupSymbol(ptr2);
				   if (pSymbol==NULL) {
					   PFOutput::EnableOneLineForceScreenOutput();
					   PFPrintfStderr("Unknown variable name on line %d",m_nInstrPtr+1);
					   end=true;
					   break;
				   }
   	
				   PFString tStr=pSymbol->GetStringValue();

				   m_sCurrentExpression=tStr;
			   }
			   Integer *toPrp=ex_evaluate(m_pTable,scrPtr);
			   if (toPrp==NULL) {
				   *pbIntValid=false;
				   m_sCurrentExpression="";
				   PFOutput::EnableOneLineForceScreenOutput();
				   PFPrintfStderr("Bad expression to PRP on line %d",m_nInstrPtr);
				   end=true;
			   }
			   *pInt=*toPrp;
			   if (!bText) {
				   valptr=pInt->Itoa();
				   m_sCurrentExpression=valptr;
				   delete[] valptr;
			   }
			   *pbIntValid=true;
			   delete toPrp;
         }
		} else if (doCommand(buff,scrPtr)) 
			end=true;
	} while (!doTest && !end);

	sLine=m_sCurrentExpression;

	if (end) {
		m_bEOF=true;
		return e_eof;
	}

	return e_ok;
}

void PFScriptFile::CurrentNumberIsPRPOrPrime(bool bIsPRP, bool bIsPrime, bool *p_bMessageStringIsValid, PFString * /*p_MessageString*/) 
{
   IPFSymbol *pSymbol=m_pTable->LookupSymbol("ISPRP");
   if (pSymbol==NULL || pSymbol->GetSymbolType()!=INTEGER_SYMBOL_TYPE) {
	   PFOutput::EnableOneLineForceScreenOutput();
	   PFPrintfStderr("Internal error, ISPRP not set correctly.");
   } else {
	   PFIntegerSymbol *pVar=(PFIntegerSymbol *)pSymbol;
	   Integer *iVal;
	   if (bIsPRP)
		   iVal=new Integer(1);
	   else
		   iVal=new Integer(0);

	   iVal=pVar->SetValue(iVal);
	   delete iVal;
   }

   pSymbol=m_pTable->LookupSymbol("ISPRIME");
   if (pSymbol==NULL || pSymbol->GetSymbolType()!=INTEGER_SYMBOL_TYPE) {
	   PFOutput::EnableOneLineForceScreenOutput();
	   PFPrintfStderr("Internal error, ISPRIME not set correctly.");
   } else {
	   PFIntegerSymbol *pVar=(PFIntegerSymbol *)pSymbol;
	   Integer *iVal;
	   if (bIsPrime)
		   iVal=new Integer(1);
	   else
		   iVal=new Integer(0);

	   iVal=pVar->SetValue(iVal);
	   delete iVal;
   }

   if (p_bMessageStringIsValid)
	   *p_bMessageStringIsValid=false;
}

	// SeekToLine is not supported, but must override to stop it seeking about the file!
int PFScriptFile::SeekToLine(int /*LineNumber*/) 
{
	m_bEOF = true;
	return e_eof;
}

