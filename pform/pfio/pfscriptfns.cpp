#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "pfiopch.h"
#include "pfscriptfile.h"
#if defined (_MSC_VER)
#include <process.h>
#endif

extern char g_ModularSieveString[256];
extern int g_nIterationCnt;

bool CheckForFatalError(const char *caller, GWInteger *gwX, int currentIteration, int maxIterations, int fftSize);

#ifndef _MSC_VER
Integer *ex_evaluate(PFSymbolTable *pContext,const PFString &e);
Integer *ex_evaluate(PFSymbolTable *pContext,const PFString &e,int m);
#endif

// these are used by SCRIPTFile's since they do not have access to the "global" 
uint64_t g_MinStartingPrimeToFactor, g_MaxStoppingPrimeToFactor;

char *PFScriptFile::FindVarName(char *varname)
{
   if (!isalpha(varname[0]) && varname[0] != '_') {
      PFOutput::EnableOneLineForceScreenOutput();
      PFPrintfStderr("Script-Error: Invalid variable name on line %d\n",m_nInstrPtr+1);
      return NULL;
   }
   uint32_t max = (uint32_t) strlen(varname);
   for (uint32_t i=1;i<max;i++) {
      if (!isalnum(varname[i]) && varname[i] != '_')
         varname[i]=0;
   }
   return varname;
}

bool PFScriptFile::BadCommand()
{
   PFOutput::EnableOneLineForceScreenOutput();
   PFPrintfStderr("Script-Error: Bad command on line %d\n",m_nInstrPtr+1);
   return true;
}

bool PFScriptFile::doCommand(char *cmd,char *args)
{
   bool end=false;
   int  i;
   uint32_t oldIterCnt = g_nIterationCnt;

   g_nIterationCnt = 0;

   // Remove trailing spaces
   i = (int) strlen(args);
   while (i)
   {
      if (args[i-1] != ' ')
         break;

      args[--i] = 0;
   }

   // putting the commands into a switch (based on the first char) allows us to
   // significantly speed up processing, due to much fewer strcmp's being done
   // per each command.
   switch (*cmd)
   {
      case 0:
      case ':':
         // a comment (or blank line), simply ignore it.
         // DO NOTHING.
         break;
      case 'C':
         if (!strcmp(cmd, "CLOSEFILE"))
            end = !CloseFile(args);
         else
            end = BadCommand();
         break;
      case 'D':
         if (!strcmp(cmd, "DIM"))
            end = !Dim(args);
         else if (!strcmp(cmd, "DIMS"))
            end = !DimS(args);
         else
            end = BadCommand();
         break;
      case 'E':
         if (!strcmp(cmd, "END"))
            end = true;
         else
            end = BadCommand();
         break;
      case 'F':
         if (!strcmp(cmd, "FACTORIZE"))
            end = !Factorize(args);
         else
            end = BadCommand();
         break;
      case 'G':
         if (!strcmp(cmd, "GOTO"))
            end = !Goto(args);
         else if (!strcmp(cmd, "GOSUB"))
            end = !Gosub(args);
         else if (!strcmp(cmd, "GETNEXT"))
            end = !GetNext(args);
         else
            end = BadCommand();
         break;
      case 'I':
         if (!strcmp(cmd, "IF"))
            end = !If(args);
         else
            end = BadCommand();
         break;
      case 'L':
         if (!strcmp(cmd, "LABEL"))
            ; // DO NOTHING
         else
            end = BadCommand();
         break;
      case 'O':
         if (!strcmp(cmd, "OPENFILEIN"))
            end = !OpenFile(args);
         else if (!strcmp(cmd, "OPENFILE"))
            end = !OpenFile(args);
         else if (!strcmp(cmd, "OPENFILEOUT"))
            end = !OpenOutFile(args);
         else if (!strcmp(cmd, "OPENFILEAPP"))
            end = !OpenAppFile(args);
         else
            end = BadCommand();
         break;
      case 'P':
         if (!strcmp(cmd, "PRINT"))
            end = !Print(args);
         else if (!strcmp(cmd, "POWMOD"))
            end = !Powmod(args);
         else
            end = BadCommand();
         break;
      case 'R':
         if (!strcmp(cmd, "RETURN"))
            end = !Return(args);
         else
            end = BadCommand();
         break;
      case 'S':
         if (!strcmp(cmd, "SET"))
            end = !Set(args);
         else if (!strcmp(cmd, "SETS"))
            end = !SetS(args);
         else if (!strcmp(cmd, "STRTOINT"))
            end = !StrToInt(args);
         else if (!strcmp(cmd, "SHELL"))
            end = !Shell(args);
         else
            end = BadCommand();
         break;
      case 'W':
         if (!strcmp(cmd, "WRITE"))
            end = !Write(args);
         else
            end = BadCommand();
         break;
      default:
         end = BadCommand();
         break;
   }
   g_nIterationCnt = oldIterCnt;

   return end;
}

bool PFScriptFile::Dim(char *args) {
   char *val,*varname;

   if (args==NULL) {
      PFOutput::EnableOneLineForceScreenOutput();
      PFPrintfStderr("Script-Error: Invalid variable name on line %d\n",m_nInstrPtr+1);
      return false;
   }
   varname=strtok(args,",");
   val=strtok(NULL,"");

   if (varname==NULL) {
      PFOutput::EnableOneLineForceScreenOutput();
      PFPrintfStderr("Script-Error: Invalid variable name on line %d\n",m_nInstrPtr+1);
      return false;
   }

   if ( (varname = FindVarName(varname)) == NULL)
      return false;

   IPFSymbol *test=m_pTable->LookupSymbol(varname);
   if (test!=NULL) {
      PFOutput::EnableOneLineForceScreenOutput();
      PFPrintfStderr("Script-Error: Variable name already in use on line %d\n",m_nInstrPtr+1);
      return false;
   }

   PFIntegerSymbol *pNew;
   Integer *iVal=NULL;

   if (val!=NULL) {
      iVal=ex_evaluate(m_pTable, val);
      if (iVal==NULL) {
         PFOutput::EnableOneLineForceScreenOutput();
         PFPrintfStderr("Script-Error: Bad initialisation on line %d, using 0\n",m_nInstrPtr+1);
      }
   }
   if (iVal==NULL)
      iVal=new Integer(0);

   pNew=new PFIntegerSymbol(varname,iVal);
   m_pTable->AddSymbol(pNew);

   return true;
}

bool PFScriptFile::DimS(char *args) {
   char *val,*varname;

   if (args==NULL) {
      PFOutput::EnableOneLineForceScreenOutput();
      PFPrintfStderr("Script-Error: Invalid variable name on line %d\n",m_nInstrPtr+1);
      return false;
   }

   varname=strtok(args,",");
   val=strtok(NULL,"");

   if (varname==NULL) {
      PFOutput::EnableOneLineForceScreenOutput();
      PFPrintfStderr("Script-Error: Invalid variable name on line %d\n",m_nInstrPtr+1);
      return false;
   }

   if ( (varname = FindVarName(varname)) == NULL)
      return false;

   IPFSymbol *test=m_pTable->LookupSymbol(varname);
   if (test!=NULL) {
      PFOutput::EnableOneLineForceScreenOutput();
      PFPrintfStderr("Script-Error: Variable name already in use on line %d\n",m_nInstrPtr+1);
      return false;
   }

   PFString sVal;
   if (val != NULL)
      sVal = val;

   m_pTable->AddSymbol(new PFStringSymbol(varname,sVal));

   return true;
}

bool PFScriptFile::Set(char *args)
{
   char *val,*varname;

   if (args==NULL) {
      PFOutput::EnableOneLineForceScreenOutput();
      PFPrintfStderr("Script-Error: Invalid variable name on line %d\n",m_nInstrPtr+1);
      return false;
   }

   varname=strtok(args,",");
   val=strtok(NULL,"");

   if (varname==NULL) {
      PFOutput::EnableOneLineForceScreenOutput();
      PFPrintfStderr("Script-Error: Invalid variable name on line %d\n",m_nInstrPtr+1);
      return false;
   }
   if (val==NULL) {
      PFOutput::EnableOneLineForceScreenOutput();
      PFPrintfStderr("Script-Error: No value to set variable to on line %d\n",m_nInstrPtr+1);
      return false;
   }

   if ( (varname = FindVarName(varname)) == NULL)
      return false;

   IPFSymbol *pSymbol=m_pTable->LookupSymbol(varname);
   if (pSymbol==NULL || pSymbol->GetSymbolType()!=INTEGER_SYMBOL_TYPE) {
      PFOutput::EnableOneLineForceScreenOutput();
      PFPrintfStderr("Script-Error: Unknown variable name on line %d\n",m_nInstrPtr+1);
      return false;
   }

   PFIntegerSymbol *pVar=(PFIntegerSymbol *)pSymbol;
   Integer *iVal=ex_evaluate(m_pTable, val);

   if (iVal==NULL) {
      PFOutput::EnableOneLineForceScreenOutput();
      PFPrintfStderr("Script-Error: Bad value on line %d\n",m_nInstrPtr+1+1);
      return false;
   }

   iVal=pVar->SetValue(iVal);
   delete iVal;

   return true;
}

bool PFScriptFile::SetS(char *args)
{

   char *val,*varname;

   if (args==NULL) {
      PFOutput::EnableOneLineForceScreenOutput();
      PFPrintfStderr("Script-Error: Invalid variable name on line %d\n",m_nInstrPtr+1);
      return false;
   }

   varname=strtok(args,",");
   val=strtok(NULL,"");

   if (varname==NULL) {
      PFOutput::EnableOneLineForceScreenOutput();
      PFPrintfStderr("Script-Error: Invalid variable name on line %d\n",m_nInstrPtr+1);
      return false;
   }
   if (val==NULL) {
      PFOutput::EnableOneLineForceScreenOutput();
      PFPrintfStderr("Script-Error: No value to set variable to on line %d\n",m_nInstrPtr+1);
      return false;
   }

   if ( (varname = FindVarName(varname)) == NULL)
      return false;

   IPFSymbol *pSymbol=m_pTable->LookupSymbol(varname);
   if (pSymbol==NULL || pSymbol->GetSymbolType()!=STRING_SYMBOL_TYPE) {
      PFOutput::EnableOneLineForceScreenOutput();
      PFPrintfStderr("Script-Error: Unknown variable name on line %d\n",m_nInstrPtr+1);
      return false;
   }

   PFStringSymbol *pVar=(PFStringSymbol *)pSymbol;
   char *ptr,*valptr;
   char *out_buff=new char[1000000];   // Allow for long decimal expansions.
   char *buffVal=new char[strlen(val)+1];
   char *param=NULL;
   PFString tStr;

   strcpy(buffVal,val);

   ptr = strchr(buffVal, ';');
   while (ptr)
   {
      // NOTE we have to allow for "escaped" ';' chars (i.e. \;  ) to still live in the string.
      if (ptr[-1] == '\\')
         ptr = strchr(&ptr[1], ';');
      else
      {
         *ptr = 0;   // this is the end of the string, and start of params.
         param = &buffVal[ptr-buffVal];
         ++param;
         param = strtok(param, ";");

         // now break us out of this loop, since we found the params.
         ptr = 0;
      }
   }

   char *iptr = buffVal;
   char *optr = out_buff;
   while (*iptr)
   {
      if (*iptr != '%')
      {
         // Allow for a single \ to escape chars.  Also allow for \n processing.
         if (*iptr == '\\' && iptr[1])
         {
            switch(iptr[1])
            {
               case 'a': *optr++ = '\a'; break; // Bell
               case 'b': *optr++ = '\b'; break; // Backspace
               case 'f': *optr++ = '\f'; break; // Form Feed
               case 'n': *optr++ = '\n'; break; // New line
               case 'r': *optr++ = '\r'; break; // Carriage return
               case 't': *optr++ = '\t'; break; // tab
               case 'v': *optr++ = '\v'; break; // Verticle tab
               case '\"': *optr++ = '\"'; break;   // Double Quote
               case '\'': *optr++ = '\''; break;   // Single Quote
               case '\\': *optr++ = '\\'; break;   // Backslash
               case '\?': *optr++ = '\?'; break;   // Literal Quote
               default:
               {
                  if ( (iptr[1] == 'x' || iptr[1] == 'X') && isxdigit(iptr[2]))
                  {
                     iptr += 2;
                     char Hex[20];
                     for (int i = 0; i < 19; ++i)
                     {
                        if (isxdigit(*iptr))
                           Hex[i] = *iptr++;
                        else
                        {
                           Hex[i] = 0;
                           break;
                        }
                     }
                     Hex[19] = 0;
                     sprintf (optr++, "%c", (char) strtol(Hex, NULL, 16));
                     iptr -= 2;
                  }
                  else if (isdigit(iptr[1]) && iptr[1] != '8' && iptr[1] != '9' && isdigit(iptr[2]) && iptr[2] != '8' && iptr[2] != '9' && isdigit(iptr[3]) && iptr[3] != '8' && iptr[3] != '9')
                  {
                     char Oct[4];
                     strncpy(Oct, &iptr[1], 3);
                     Oct[3] = 0;
                     sprintf (optr++, "%c", (char) strtol(Oct, NULL, 8));
                     iptr += 2;
                  }
                  else
                  {
                     // Microsoft specific type behavior, i.e. simply copy the escaped char
                     *optr++ = iptr[1];
                  }
               }
            }
            iptr += 2;
         }
         else
            *optr++ = *iptr++;
      }
      else
      {
         // This is a %s  %d or %% (or there is an error).  We output a string, int or '%'
         if (iptr[1] == '%')
            *optr++ = '%';
         else
         {
            if (param == NULL)
            {
               PFOutput::EnableOneLineForceScreenOutput();
               PFPrintfStderr("Script-Error: Not enough arguments in string on line %d\n",m_nInstrPtr+1);
               delete[] buffVal;
               delete[] out_buff;
               return false;
            }
            switch(iptr[1])
            {
               case 's':
               {
                  pSymbol=m_pTable->LookupSymbol(param);
                  if (pSymbol==NULL)
                  {
                     // simply output the paramter "litterally".  i.e. the string itself.
                     strcpy(optr, param);
                     optr += strlen(param);
                  }
                  else
                  {
                     if (pSymbol->GetSymbolType()==STRING_SYMBOL_TYPE)
                     {
                        // found a string var, so print out it's contents.
                        tStr=((PFStringSymbol *)pSymbol)->GetStringValue();
                        strcpy(optr,tStr);
                        optr += strlen(tStr);
                     }
                     else
                     {
                        PFOutput::EnableOneLineForceScreenOutput();
                        PFPrintfStderr("Script-Error: Invalid argument (variable not a string, but used an %%s) %d\n",iptr[1], m_nInstrPtr+1);
                        delete[] buffVal;
                        delete[] out_buff;
                        return false;
                     }
                  }
                  break;
               }
               case 'd':
               {
                  Integer *tVal=ex_evaluate(m_pTable,param);
                  if (tVal==NULL) {
                     PFOutput::EnableOneLineForceScreenOutput();
                     PFPrintfStderr("Script-Error: Couldn't evaluate argument on line %d\n",m_nInstrPtr+1);
                     delete[] buffVal;
                     delete[] out_buff;
                     return false;
                  }
                  valptr=tVal->Itoa();
                  strcpy(optr,valptr);
                  optr += strlen(valptr);
                  delete[] valptr;
                  delete tVal;
                  break;
               }
               default:
                  PFOutput::EnableOneLineForceScreenOutput();
                  PFPrintfStderr("Script-Error: Invalid argument type (do not know how to handle a %%%c) on line %d\n",iptr[1], m_nInstrPtr+1);
                  delete[] buffVal;
                  delete[] out_buff;
                  return false;
            }
            // Get ready for next param (if there is one).
            param = strtok(NULL, ";");
         }
         iptr += 2;
      }
   }
   *optr = 0;  //  null terminate the string.
   tStr=out_buff;
   // PFStringSymbols do not need to have the return deleted, like the PFIntegerSymbol's do.
   pVar->SetValue(tStr);

   delete[] buffVal;
   delete[] out_buff;

   return true;
}

bool PFScriptFile::StrToInt(char *args) {
   char *val,*varname;

   if (args==NULL) {
      PFOutput::EnableOneLineForceScreenOutput();
      PFPrintfStderr("Script-Error: Invalid variable name on line %d\n",m_nInstrPtr+1);
      return false;
   }

   varname=strtok(args,",");
   val=strtok(NULL,",");

   if (varname==NULL) {
      PFOutput::EnableOneLineForceScreenOutput();
      PFPrintfStderr("Script-Error: Invalid variable name on line %d\n",m_nInstrPtr+1);
      return false;
   }
   if (val==NULL) {
      PFOutput::EnableOneLineForceScreenOutput();
      PFPrintfStderr("Script-Error: No value to set variable to on line %d\n",m_nInstrPtr+1);
      return false;
   }

   if ( (varname = FindVarName(varname)) == NULL)
      return false;

   IPFSymbol *pSymbol=m_pTable->LookupSymbol(varname);
   if (pSymbol==NULL || pSymbol->GetSymbolType()!=INTEGER_SYMBOL_TYPE) {
      PFOutput::EnableOneLineForceScreenOutput();
      PFPrintfStderr("Script-Error: Unknown variable name on line %d\n",m_nInstrPtr+1);
      return false;
   }
   IPFSymbol *pValue=m_pTable->LookupSymbol(val);
   if (pValue==NULL || pValue->GetSymbolType()!=STRING_SYMBOL_TYPE) {
      PFOutput::EnableOneLineForceScreenOutput();
      PFPrintfStderr("Script-Error: Unknown variable name on line %d\n",m_nInstrPtr+1);
      return false;
   }
   PFString tStr;
   tStr=((PFStringSymbol *)pValue)->GetStringValue();
   val = new char[tStr.GetLength()+1];
   strcpy(val, tStr);

   PFIntegerSymbol *pVar=(PFIntegerSymbol *)pSymbol;

   Integer *iVal=ex_evaluate(m_pTable, val);
   delete[] val;

   if (iVal==NULL) {
      PFOutput::EnableOneLineForceScreenOutput();
      PFPrintfStderr("Script-Error: Bad value on line %d\n",m_nInstrPtr+1+1);
      return false;
   }
   iVal=pVar->SetValue(iVal);
   delete iVal;

   return true;
}

bool PFScriptFile::Powmod(char *args) {
   char *val1, *val2, *val3,*varname;

   if (args==NULL) {
      PFOutput::EnableOneLineForceScreenOutput();
      PFPrintfStderr("Script-Error: Invalid variable name on line %d\n",m_nInstrPtr+1);
      return false;
   }

   varname=strtok(args,",");
   val1=strtok(NULL,",");
   val2=strtok(NULL,",");
   val3=strtok(NULL,"");

   if (varname==NULL) {
      PFOutput::EnableOneLineForceScreenOutput();
      PFPrintfStderr("Script-Error: Invalid variable name on line %d\n",m_nInstrPtr+1);
      return false;
   }
   if (val1==NULL) {
      PFOutput::EnableOneLineForceScreenOutput();
      PFPrintfStderr("Script-Error: No value #1 to set variable to on line %d\n",m_nInstrPtr+1);
      return false;
   }
   if (val2==NULL) {
      PFOutput::EnableOneLineForceScreenOutput();
      PFPrintfStderr("Script-Error: No value #2 to set variable to on line %d\n",m_nInstrPtr+1);
      return false;
   }
   if (val2==NULL) {
      PFOutput::EnableOneLineForceScreenOutput();
      PFPrintfStderr("Script-Error: No value #3 to set variable to on line %d\n",m_nInstrPtr+1);
      return false;
   }

   if ( (varname = FindVarName(varname)) == NULL)
      return false;

   IPFSymbol *pSymbol=m_pTable->LookupSymbol(varname);
   if (pSymbol==NULL || pSymbol->GetSymbolType()!=INTEGER_SYMBOL_TYPE) {
      PFOutput::EnableOneLineForceScreenOutput();
      PFPrintfStderr("Script-Error: Unknown variable name on line %d\n",m_nInstrPtr+1);
      return false;
   }

   Integer b, e, n;

   IPFSymbol *pValue=m_pTable->LookupSymbol(val1);
   if (pValue && pValue->GetSymbolType()==INTEGER_SYMBOL_TYPE) {
      b = *((PFIntegerSymbol*)pValue)->GetValue();
   }
   else {
      Integer *iVal=NULL;
      iVal=ex_evaluate(m_pTable, val1);
      if (iVal==NULL) {
         PFOutput::EnableOneLineForceScreenOutput();
         PFPrintfStderr("Script-Error: Bad initialisation (param #1) on line %d, using 0\n",m_nInstrPtr+1);
         return false;
      }
      b = *iVal;
      delete iVal;
   }

   pValue=m_pTable->LookupSymbol(val2);
   if (pValue && pValue->GetSymbolType()==INTEGER_SYMBOL_TYPE) {
      e = *((PFIntegerSymbol*)pValue)->GetValue();
   }
   else {
      Integer *iVal=NULL;
      iVal=ex_evaluate(m_pTable, val2);
      if (iVal==NULL) {
         PFOutput::EnableOneLineForceScreenOutput();
         PFPrintfStderr("Script-Error: Bad initialisation (param #2)  on line %d, using 0\n",m_nInstrPtr+1);
         return false;
      }
      e = *iVal;
      delete iVal;
   }

   pValue=m_pTable->LookupSymbol(val3);
   if (pValue && pValue->GetSymbolType()==INTEGER_SYMBOL_TYPE) {
      n = *((PFIntegerSymbol*)pValue)->GetValue();
   }
   else {
      Integer *iVal=NULL;
      iVal=ex_evaluate(m_pTable, val3);
      if (iVal==NULL) {
         PFOutput::EnableOneLineForceScreenOutput();
         PFPrintfStderr("Script-Error: Bad initialisation (param #3)  on line %d, using 0\n",m_nInstrPtr+1);
         return false;
      }
      n = *iVal;
      delete iVal;
   }

   PFIntegerSymbol *pVar=(PFIntegerSymbol *)pSymbol;
   Integer *iVal = new Integer;

   // For small numbers, use GMP for the expmod.  For large ones, use gwnum.
   if (numbits(n) < 650)
      mpz_powm( iVal->gmp(), b.gmp(), e.gmp(), n.gmp());
   else
      GWPowMod(iVal, b, e, n, val3);

   iVal=pVar->SetValue(iVal);
   delete iVal;

   return true;
}

void PFScriptFile::GWPowMod(Integer *result, Integer b, Integer e, Integer n, char *modulus)
{
   int      i, s, iterations;

   gwinit2(&gwdata, sizeof(gwhandle), (char *) GWNUM_VERSION);

   CreateModulus(&n, modulus);

   gwsetmaxmulbyconst(&gwdata, GWMULBYCONST_MAX);  // maximum multiplier
   gwsetmulbyconst(&gwdata, 1);

   GWInteger gwX, gwY;

   gwX = b;
   gwY = 1;
   iterations = s = numbits(e);

   i = 0;
   for (;;)
   {
      if (e & 1)
      {
         gw_clear_maxerr(&gwdata);

         if (s < 30 || i < 30)
         {
            gwsetnormroutine(&gwdata, 0, 1, 0);
            gwmul_carefully(gwX, gwY);
         }
         else
         {
            gwsetnormroutine(&gwdata, 0, 0, 0);
            gwmul(gwX, gwY);
         }

         CheckForFatalError("GWPowMod", &gwX, i, s, 1); 
         CheckForFatalError("GWPowMod", &gwY, i, s, 1); 
      }

      e >>= 1;
      if (e == 0)
      {
         *result = gwY;
         DestroyModulus();
         return;
      }
      
      gw_clear_maxerr(&gwdata);

      if (s < 30 || i < 30)
      {
         gwsetnormroutine(&gwdata, 0, 1, 0);
         gwsquare2_carefully(gwX);
      }
      else
      {
         gwsetnormroutine(&gwdata, 0, 0, 0);
         gwsquare2(gwX);
      }

      i++;
      s--;

      if (i > 0 && i % 2500 == 0)
      {
         PFPrintfStderr("GWPowMod: %d/%d\r", i, iterations);
         PFfflush(stderr);
      }
   }
}

bool PFScriptFile::Goto(char *args) {
   int newLine;

   while (isspace(*args))
      args++;

   if (isdigit(*args))
      newLine=m_nInstrPtr+atoi(args)-1;
   else {
      int i=0;
      while (args[i] == '_' || isalnum(args[i]))
         i++;
      args[i]=0;
      IPFSymbol *pSymbol=m_pTable->LookupSymbol(args);
      if (pSymbol==NULL || pSymbol->GetSymbolType()!=INT_SYMBOL_TYPE) {
         PFOutput::EnableOneLineForceScreenOutput();
         PFPrintfStderr("Script-Error: Bad label in goto on line %d\n",m_nInstrPtr+1);
         return false;
      }
      newLine=((PFIntSymbol *)pSymbol)->GetValue();
   }

   if (newLine<0 || newLine>=m_nNumLines) {
      PFOutput::EnableOneLineForceScreenOutput();
      PFPrintfStderr("Script-Error: Bad line number in goto on line %d\n",m_nInstrPtr+1);
      return false;
   }

   m_nInstrPtr=newLine;
   return true;
}

bool PFScriptFile::Gosub(char *args) {
   int newLine;

   if (m_GosubLevel == 65535) {
      PFOutput::EnableOneLineForceScreenOutput();
      PFPrintfStderr("Script-Error: Gosub Max-depth reached  on line %d\n",m_nInstrPtr+1);
      return false;
   }

   while (isspace(*args))
      args++;

   int i=0;
   while (args[i] == '_' || isalnum(args[i]))
      i++;
   args[i]=0;
   IPFSymbol *pSymbol=m_pTable->LookupSymbol(args);
   if (pSymbol==NULL || pSymbol->GetSymbolType()!=INT_SYMBOL_TYPE) {
      PFOutput::EnableOneLineForceScreenOutput();
      PFPrintfStderr("Script-Error: Bad label in goto on line %d\n",m_nInstrPtr+1);
      return false;
   }
   newLine=((PFIntSymbol *)pSymbol)->GetValue();

   if (newLine<0 || newLine>=m_nNumLines) {
      PFOutput::EnableOneLineForceScreenOutput();
      PFPrintfStderr("Script-Error: Bad line number in gosub on line %d\n",m_nInstrPtr+1);
      return false;
   }

   char GosubRetLabel[40];
   sprintf (GosubRetLabel, "_%d_GoSubLabel_#", m_GosubLevel++);
   PFIntSymbol *pLabel=new PFIntSymbol(GosubRetLabel,m_nInstrPtr);
   m_pTable->AddSymbol(pLabel);

   m_nInstrPtr=newLine;
   return true;
}

bool PFScriptFile::Return(char *args) {
   int newLine;

   if (m_GosubLevel == 0) {
      PFOutput::EnableOneLineForceScreenOutput();
      PFPrintfStderr("Script-Error: Return without gosub on line %d\n",m_nInstrPtr+1);
      return false;
   }

   while (isspace(*args))
      args++;

   int i=0;
   while (isdigit(args[i]))
      i++;
   args[i]=0;

   int RetDepth = 1;
   if (*args)
      RetDepth = atoi(args);
   if (!RetDepth)
      RetDepth = 1;
   if (RetDepth == -1)
      RetDepth = m_GosubLevel;

   if (m_GosubLevel < RetDepth) {
      PFOutput::EnableOneLineForceScreenOutput();
      PFPrintfStderr("Script-Error: Trying to return %d with gosub level %d on line %d\n",RetDepth, m_GosubLevel,m_nInstrPtr+1);
      return false;
   }

   char GosubRetLabel[40];
   m_GosubLevel -= RetDepth;
   sprintf (GosubRetLabel, "_%d_GoSubLabel_#", m_GosubLevel);
   IPFSymbol *pSymbol=m_pTable->LookupSymbol(GosubRetLabel);
   if (pSymbol==NULL || pSymbol->GetSymbolType()!=INT_SYMBOL_TYPE) {
      PFOutput::EnableOneLineForceScreenOutput();
      PFPrintfStderr("Script-Error: Could not find Return point %s on line %d\n",GosubRetLabel,m_nInstrPtr+1);
      return false;
   }
   newLine=((PFIntSymbol *)pSymbol)->GetValue();

   if (newLine<0 || newLine>=m_nNumLines) {
      PFOutput::EnableOneLineForceScreenOutput();
      // don't print warning if returning to 1 line past the end of the file. That is
      // "valid", as a GOSUB being the last line can happen, and causes this condition.
      if (newLine != m_nNumLines)
         PFPrintfStderr("Script-Error: Bad line number in return on line %d\n",m_nInstrPtr+1);
      return false;
   }
   m_nInstrPtr=newLine;
   return true;
}

bool PFScriptFile::Factorize(char *args) {
   char *varname;

   if (args==NULL) {
      PFOutput::EnableOneLineForceScreenOutput();
      PFPrintfStderr("Script-Error: Invalid variable name on line %d\n",m_nInstrPtr+1);
      return false;
   }

   varname=strtok(args,",");

   if (varname==NULL) {
      PFOutput::EnableOneLineForceScreenOutput();
      PFPrintfStderr("Script-Error: Invalid variable name on line %d\n",m_nInstrPtr+1);
      return false;
   }

   // We DO NOT check for a valid var name. We want to also allow expressions to
   // "pass" through, which could easily start with a ( or a number.
// if ( (varname = FindVarName(varname)) == NULL)
//    return false;

   Integer *nVal=NULL;

   IPFSymbol *pSymbol=m_pTable->LookupSymbol(varname);
   if (pSymbol && pSymbol->GetSymbolType()==INTEGER_SYMBOL_TYPE) {
      PFIntegerSymbol *pSymbolN = (PFIntegerSymbol *)pSymbol;
      nVal = new Integer(* (pSymbolN->GetValue()));
   }
   else {
      nVal=ex_evaluate(m_pTable, varname);
      if (nVal==NULL) {
         PFOutput::EnableOneLineForceScreenOutput();
         PFPrintfStderr("Script-Error: Bad expression to FACTORIZE on line %d",m_nInstrPtr+1);
         return false;
      }
   }
   // We have to add _N to the symbol table (since it is not "normally" there.
   m_pTable->AddSymbol(new PFIntegerSymbol("_N",nVal));

   // We are modifying our "local" symbol table, so setting _PMAX and _PMIN do not impact the PRP
   // stage.  HOWEVER, setting the modular value DOES impact the global, so we have to save it,
   // and restore it, if we modify it.
   bool bTempM=false;
   char OldM[256];

   IPFSymbol *pSymbolTmp=m_pTable->LookupSymbol("MAXF");
   if (pSymbolTmp!=NULL && pSymbolTmp->GetSymbolType()==INTEGER_SYMBOL_TYPE && *((PFIntegerSymbol*)pSymbolTmp)->GetValue() > 0) {
      Integer *I = new Integer(*((PFIntegerSymbol*)pSymbolTmp)->GetValue());
      PFIntegerSymbol *pNew=new PFIntegerSymbol("_PMAX",I);
      m_pTable->AddSymbol(pNew);
   }
   else if (g_MaxStoppingPrimeToFactor)
   {
      PFIntegerSymbol *pNew=new PFIntegerSymbol("_PMAX",new Integer(g_MaxStoppingPrimeToFactor));
      m_pTable->AddSymbol(pNew);
   }
   pSymbolTmp=m_pTable->LookupSymbol("MINF");
   if (pSymbolTmp!=NULL && pSymbolTmp->GetSymbolType()==INTEGER_SYMBOL_TYPE && *((PFIntegerSymbol*)pSymbolTmp)->GetValue() > 0) {
      Integer *I = new Integer(*((PFIntegerSymbol*)pSymbolTmp)->GetValue());
      PFIntegerSymbol *pNew=new PFIntegerSymbol("_PMIN",I);
      m_pTable->AddSymbol(pNew);
   }
   else if (g_MinStartingPrimeToFactor)
   {
      PFIntegerSymbol *pNew=new PFIntegerSymbol("_PMIN",new Integer(g_MinStartingPrimeToFactor));
      m_pTable->AddSymbol(pNew);
   }

   pSymbolTmp=m_pTable->LookupSymbol("MODF");
   if (pSymbolTmp!=NULL && pSymbolTmp->GetSymbolType()==STRING_SYMBOL_TYPE && ((PFStringSymbol*)pSymbolTmp)->GetStringValue() != PFString("")) {
      bTempM = true;
      strcpy(OldM, g_ModularSieveString);
      strcpy(g_ModularSieveString, ((PFStringSymbol*)pSymbolTmp)->GetStringValue());
   }

   pSymbolTmp=m_pTable->LookupSymbol("FACTORFOUND");
   if (pSymbolTmp==NULL || pSymbolTmp->GetSymbolType()!=INTEGER_SYMBOL_TYPE) {
      PFOutput::EnableOneLineForceScreenOutput();
      PFPrintfStderr("Internal error, FACTORFOUND not set correctly.");
   }
   PFIntegerSymbol *pSymbolFF = (PFIntegerSymbol *)pSymbolTmp;

   pSymbolTmp=m_pTable->LookupSymbol("ISPRP");
   if (pSymbolTmp==NULL || pSymbolTmp->GetSymbolType()!=INTEGER_SYMBOL_TYPE) {
      PFOutput::EnableOneLineForceScreenOutput();
      PFPrintfStderr("Internal error, ISPRP not set correctly.");
   }

   pSymbolTmp=m_pTable->LookupSymbol("ISPRIME");
   if (pSymbolTmp==NULL || pSymbolTmp->GetSymbolType()!=INTEGER_SYMBOL_TYPE) {
      PFOutput::EnableOneLineForceScreenOutput();
      PFPrintfStderr("Internal error, ISPRIME not set correctly.");
   }
   PFIntegerSymbol *pSymbolPR = (PFIntegerSymbol *)pSymbolTmp;

   Integer *iVal;
   iVal=new Integer(0);
   iVal = pSymbolPR->SetValue(iVal);
   delete iVal;
   iVal=new Integer(1);
   iVal = pSymbolFF->SetValue(iVal);
   delete iVal;

   // Do the factorization here!

   /////////////Code from PFGW_MAIN//////////////

   PFFactorizationSymbol *pffN;
   m_pTable->AddSymbol(pffN=new PFFactorizationSymbol("_NFACTOR"));
   m_pTable->AddSymbol(new PFStringSymbol("_SN","_NFACTOR"));

   if (g_ModularSieveString[0])
   {
      m_pTable->AddSymbol(new PFStringSymbol("_USEMODFACTOR","_USEMODFACTOR"));
      m_pTable->AddSymbol(new PFStringSymbol("_MODFACTOR",g_ModularSieveString));
   }
   PFFunctionSymbol::CallSubroutine("@factor",m_pTable);

   PFString sFactorization;
   sFactorization=pffN->GetStringValue();
   IPFSymbol *pQ=0;
   if(sFactorization!="1" && (pQ=m_pTable->LookupSymbol("_Q")) != NULL)
   {
      Integer _Q=*((PFIntegerSymbol*)pQ)->GetValue();

//      // Yes I know its a stupid way of finding if its a prime
      if (_Q==1 && !strchr(LPCTSTR(sFactorization), '^') && !strchr(LPCTSTR(sFactorization), '*'))
      {
         iVal=new Integer(1);
         iVal=pSymbolPR->SetValue(iVal);
         delete iVal;
      }

      else
      {
         iVal=new Integer;

         if (strchr(LPCTSTR(sFactorization), '^') || strchr(LPCTSTR(sFactorization), '*'))
         {
            char *Buf = new char [sFactorization.GetLength()+1];
            strcpy(Buf, LPCTSTR(sFactorization));
            char *cp = strchr(Buf, '^');
            if (!cp)
               cp = strchr(Buf, '*');
            *cp = 0;
            sFactorization = Buf;
            delete[] Buf;
         }
         iVal->atoI(sFactorization);
         iVal = pSymbolFF->SetValue(iVal);
         delete iVal;
      }
   }
   /////////////end Code from PFGW_MAIN//////////////

   // If we modified the mod string, then we MUST fix it back up.
   if (bTempM)
      strcpy(g_ModularSieveString, OldM);

   return true;
}

bool PFScriptFile::Print(char *args) {

   bool bAddNL=true;
   char *cp = &args[strlen(args)-1];
   if (*cp == 'c' && cp[-1] == ',') {
      cp[-1] = 0;
      bAddNL=false;
   }

   IPFSymbol *pSymbol=m_pTable->LookupSymbol(args);
   if (pSymbol==NULL) {
      PFOutput::EnableOneLineForceScreenOutput();
      PFPrintfStderr("Script-Error: Unknown variable name on line %d\n",m_nInstrPtr+1);
      return false;
   }

   PFString tStr=pSymbol->GetStringValue();

   if (bAddNL)
      tStr+="\n";

   PFOutput::EnableOneLineForceScreenOutput();
   PFPrintfLog("%s",(const char *)tStr);
   return true;
}

bool PFScriptFile::Write(char *args) {
   char *filevar,*strvar;
   FILE *fptr;

   filevar=strtok(args,",");
   if (filevar==NULL) {
      PFOutput::EnableOneLineForceScreenOutput();
      PFPrintfStderr("Script-Error: No file variable name on line %d\n",m_nInstrPtr+1);
      return false;
   }

   strvar=strtok(NULL,",");
   if (strvar==NULL) {
      PFOutput::EnableOneLineForceScreenOutput();
      PFPrintfStderr("Script-Error: No variable to write on line %d\n",m_nInstrPtr+1);
      return false;
   }

   IPFSymbol *pSymbol=m_pTable->LookupSymbol(filevar);
   if (pSymbol==NULL || pSymbol->GetSymbolType()!=OUTPUT_FILE_SYMBOL_TYPE) {
      PFOutput::EnableOneLineForceScreenOutput();
      PFPrintfStderr("Script-Error: Unknown file variable name on line %d\n",m_nInstrPtr+1);
      return false;
   }
   PFOutputFileSymbol *pOutput=(PFOutputFileSymbol *)pSymbol;
   fptr=pOutput->GetFile();

   pSymbol=m_pTable->LookupSymbol(strvar);
   if (pSymbol==NULL) {
      PFOutput::EnableOneLineForceScreenOutput();
      PFPrintfStderr("Script-Error: Unknown variable name on line %d\n",m_nInstrPtr+1);
      return false;
   }

   PFString tStr=pSymbol->GetStringValue();

   fprintf(fptr,"%s\n",(const char *)tStr);
   fflush(fptr);

   return true;
}

bool PFScriptFile::If(char *args) {
   char *val,*cmd1,*cmd2;

   val=args;
   int len = (int) strlen(args)+1;

   cmd1=strstr(args,"THEN");
   if (!cmd1)
      cmd1=strstr(args,"then");
   if (!cmd1)
      cmd1=strstr(args,"Then");
   if (!cmd1) {
      PFOutput::EnableOneLineForceScreenOutput();
      PFPrintfStderr("Script-Error: No THEN in IF statement on line %d\n",m_nInstrPtr+1);
      return false;
   }

   *cmd1=0;
   cmd1+=4;
   while (isspace(*cmd1))
      cmd1++;

   cmd2=strstr(cmd1,"ELSE");
   if (!cmd2)
      cmd2=strstr(cmd1,"else");
   if (!cmd2)
      cmd2=strstr(cmd1,"Else");
   if (cmd2) {
      *cmd2=0;
      char *cmd2_ = cmd2-1;
      while (isspace(*cmd2_))
      {
         // Eat the trailing white space from the first THEN command.  If we do not do
         // this, and a variable "ends" the THEN, it will NOT be found (i.e. the var will
         // be "VARNAME " and not VARNAME
         *cmd2_ = 0;
         --cmd2_;
      }
      cmd2+=4;
      while (isspace(*cmd2))
         cmd2++;
   }

   Integer *iVal=ex_evaluate(m_pTable, val);

   if (iVal==NULL) {
      PFOutput::EnableOneLineForceScreenOutput();
      PFPrintfStderr("Script-Error: Bad expression on line %d\n",m_nInstrPtr+1);
      return false;
   }

   bool bIsZero = !!(*iVal==0);
   delete iVal;
   if (!bIsZero) {
      char *buff = new char[len];
      char t;
      int i;
         // ok should now be looking at a command, isolate command.
      i=0;
      do {
         t=buff[i]=(char)toupper(cmd1[i]);
         if (!isalpha(t))
            buff[i]=0;
         i++;
      } while (isalpha(t));

      cmd1+=i;
      while (isspace(*cmd1))
         cmd1++;

         // command name now in buff, arguments pointed to by cmd1.
      if (doCommand(buff,cmd1))
      {
         delete[] buff;
         return false;
      }
      delete[] buff;
   } else if (cmd2!=NULL) {
      char *buff = new char[len];
      char t;
      int i;
         // ok should now be looking at a command, isolate command.
      i=0;
      do {
         t=buff[i]=(char)toupper(cmd2[i]);
         if (!isalpha(t))
            buff[i]=0;
         i++;
      } while (isalpha(t));

      cmd2+=i;
      while (isspace(*cmd2))
         cmd2++;

         // command name now in buff, arguments pointed to by cmd2.
      if(doCommand(buff,cmd2))
      {
         delete[] buff;
         return false;
      }
      delete[] buff;
   }

   return true;
}

bool PFScriptFile::OpenFile(char *args) {
   char *varname,*filename;

   varname=strtok(args,",");
   if (varname==NULL) {
      PFOutput::EnableOneLineForceScreenOutput();
      PFPrintfStderr("Script-Error: No variable name on line %d\n",m_nInstrPtr+1);
      return false;
   }

   filename=strtok(NULL,",");
   if (filename==NULL) {
      PFOutput::EnableOneLineForceScreenOutput();
      PFPrintfStderr("Script-Error: No file name on line %d\n",m_nInstrPtr+1);
      return false;
   }

   IPFSymbol *test=m_pTable->LookupSymbol(varname);
   if (test!=NULL) {
      PFOutput::EnableOneLineForceScreenOutput();
      PFPrintfStderr("Script-Error: Variable name already in use on line %d\n",m_nInstrPtr+1);
      return false;
   }

   PFSimpleFile *pFile;
   PFInputFileSymbol *pNew;
   const char *errormsg;

   IPFSymbol *FNameStr=m_pTable->LookupSymbol(filename);
   if (FNameStr && FNameStr->GetSymbolType()==STRING_SYMBOL_TYPE)
   {
       // Ok, there was a string var, vs a "literal" string.  Use the string var to open the file.
       PFString tStrFname = ((PFStringSymbol *)FNameStr)->GetStringValue();
       pFile=openInputFile(tStrFname,NULL,&errormsg);
   }
   else
        {
       pFile=openInputFile(filename,NULL,&errormsg);
   }

   if (pFile==NULL) {
      PFOutput::EnableOneLineForceScreenOutput();
      PFPrintfStderr("Script-Error: %s on line %d\n",errormsg,m_nInstrPtr+1);
      return false;
   }

   pNew=new PFInputFileSymbol(varname,pFile);

   m_pTable->AddSymbol(pNew);

   return true;
}

bool PFScriptFile::OpenOutFile(char *args) {
   char *varname,*filename;

   varname=strtok(args,",");
   if (varname==NULL) {
      PFOutput::EnableOneLineForceScreenOutput();
      PFPrintfStderr("Script-Error: No variable name on line %d\n",m_nInstrPtr+1);
      return false;
   }

   filename=strtok(NULL,",");
   if (filename==NULL) {
      PFOutput::EnableOneLineForceScreenOutput();
      PFPrintfStderr("Script-Error: No file name on line %d\n",m_nInstrPtr+1);
      return false;
   }

   IPFSymbol *test=m_pTable->LookupSymbol(varname);
   if (test!=NULL) {
      PFOutput::EnableOneLineForceScreenOutput();
      PFPrintfStderr("Script-Error: Variable name already in use on line %d\n",m_nInstrPtr+1);
      return false;
   }

   FILE *pFile;
   PFOutputFileSymbol *pNew;

   IPFSymbol *FNameStr=m_pTable->LookupSymbol(filename);
   if (FNameStr && FNameStr->GetSymbolType()==STRING_SYMBOL_TYPE)
   {
       // Ok, there was a string var, vs a "literal" string.  Use the string var to open the file.
       PFString tStrFname = ((PFStringSymbol *)FNameStr)->GetStringValue();
       pFile=pFile=fopen(tStrFname,"w");
   }
   else
        {
       pFile=pFile=fopen(filename,"w");
   }

   if (pFile==NULL) {
      PFOutput::EnableOneLineForceScreenOutput();
      PFPrintfStderr("Script-Error: Error opening file for putput on line %d\n",m_nInstrPtr+1);
      return false;
   }

   pNew=new PFOutputFileSymbol(varname,pFile);

   m_pTable->AddSymbol(pNew);

   return true;
}

bool PFScriptFile::OpenAppFile(char *args) {
   char *varname,*filename;

   varname=strtok(args,",");
   if (varname==NULL) {
      PFOutput::EnableOneLineForceScreenOutput();
      PFPrintfStderr("Script-Error: No variable name on line %d\n",m_nInstrPtr+1);
      return false;
   }

   filename=strtok(NULL,",");
   if (filename==NULL) {
      PFOutput::EnableOneLineForceScreenOutput();
      PFPrintfStderr("Script-Error: No file name on line %d\n",m_nInstrPtr+1);
      return false;
   }

   IPFSymbol *test=m_pTable->LookupSymbol(varname);
   if (test!=NULL) {
      PFOutput::EnableOneLineForceScreenOutput();
      PFPrintfStderr("Script-Error: Variable name already in use on line %d\n",m_nInstrPtr+1);
      return false;
   }

   FILE *pFile;
   PFOutputFileSymbol *pNew;

   IPFSymbol *FNameStr=m_pTable->LookupSymbol(filename);
   if (FNameStr && FNameStr->GetSymbolType()==STRING_SYMBOL_TYPE)
   {
       // Ok, there was a string var, vs a "literal" string.  Use the string var to open the file.
       PFString tStrFname = ((PFStringSymbol *)FNameStr)->GetStringValue();
       pFile=pFile=fopen(tStrFname,"a+");
   }
   else
        {
       pFile=pFile=fopen(filename,"a+");
   }

   if (pFile==NULL) {
      PFOutput::EnableOneLineForceScreenOutput();
      PFPrintfStderr("Script-Error: Error opening file for putput on line %d\n",m_nInstrPtr+1);
      return false;
   }

   pNew=new PFOutputFileSymbol(varname,pFile);

   m_pTable->AddSymbol(pNew);

   return true;
}

bool PFScriptFile::GetNext(char *args) {
   char *varname,*filevarname,*strvarname;

   varname=strtok(args,",");
   if (varname==NULL) {
      PFOutput::EnableOneLineForceScreenOutput();
      PFPrintfStderr("Script-Error: No variable name on line %d\n",m_nInstrPtr+1);
      return false;
   }

   filevarname=strtok(NULL,",");
   if (filevarname==NULL) {
      PFOutput::EnableOneLineForceScreenOutput();
      PFPrintfStderr("Script-Error: No file variable name on line %d\n", m_nInstrPtr+1);
      return false;
   }

   strvarname=strtok(NULL,",");

   IPFSymbol *pSymbol=m_pTable->LookupSymbol(varname);
   if (pSymbol==NULL || pSymbol->GetSymbolType()!=INTEGER_SYMBOL_TYPE) {
      PFOutput::EnableOneLineForceScreenOutput();
      PFPrintfStderr("Script-Error: Bad variable name <%s> on line %d\n", varname, m_nInstrPtr+1);
      return false;
   }
   PFIntegerSymbol *pIntSym=(PFIntegerSymbol *)pSymbol;

   if (*filevarname == ' ')
      strcpy(filevarname, filevarname+1);

   pSymbol=m_pTable->LookupSymbol(filevarname);
   if (pSymbol==NULL || pSymbol->GetSymbolType()!=INPUT_FILE_SYMBOL_TYPE) {
      PFOutput::EnableOneLineForceScreenOutput();
      PFPrintfStderr("Script-Error: Bad file variable name <%s> on line %d\n", filevarname, m_nInstrPtr+1);
      return false;
   }
   PFInputFileSymbol *pFileSym=(PFInputFileSymbol *)pSymbol;

   PFStringSymbol *pStrSym=NULL;
   if (strvarname) {
      if (*strvarname == ' ')
         strcpy(strvarname, strvarname+1);
      pSymbol=m_pTable->LookupSymbol(strvarname);
      if (pSymbol==NULL || pSymbol->GetSymbolType()!=STRING_SYMBOL_TYPE) {
         PFOutput::EnableOneLineForceScreenOutput();
         PFPrintfStderr("Script-Error: Bad string variable <%s> name on line %d\n", strvarname, m_nInstrPtr+1);
         return false;
      }
      pStrSym=(PFStringSymbol *)pSymbol;
   }

   Integer *iVal=new Integer();
   PFString expr;
   bool bIsValid=false;
   if (pFileSym->GetFile()->GetNextLine(expr,iVal,&bIsValid)!=PFSimpleFile::e_ok) {
      *iVal=-1;
      bIsValid=true;
      expr="EOF";
   } else {
      if (!(bIsValid)) {
         delete iVal;
         iVal=ex_evaluate(m_pTable,expr);
      }
      if (iVal==NULL) {
         PFOutput::EnableOneLineForceScreenOutput();
         PFPrintfStderr("Script-Error: Evaluator failed on line %d\n",m_nInstrPtr+1);
         return false;
      }
   }

   iVal=pIntSym->SetValue(iVal);
   delete iVal;

   if (pStrSym)
      pStrSym->SetValue(expr);  // note a string symbol does not need it's return deleted.

   return true;
}

bool PFScriptFile::CloseFile(char *args) {
   m_pTable->EraseSymbol(args);
   return true;
}

// **********************************************************************
// Function Name:   LaunchApp
// Description:     Launch an application and possibly wait for it to terminate.
// Parameters:      pszCmd              -- pointer to application to launch
//                  pszCmdLine          -- pointer to command line parameters
//                  pszCurrentDirectory -- current drive and directory for app
//                  bWait               -- true == wait for app to terminate
//                  dwMilliseconds      -- time-out interval in milliseconds
//                                         (ignored if bWait == false)
// Returns:         bool -- true if app is launched successfully
// **********************************************************************
#if defined (_MSC_VER)
#include <direct.h>  // Needed for _fullpath() call in Shell() function under VC
int LaunchApp( const char *pszCmd, const char *pszCmdLine, /* const char *pszCurrentDirectory / * = NULL * /, */ bool bWait /* = FALSE */, DWORD dwMilliseconds /* = INFINITE */ )
{
   STARTUPINFO         si;
   PROCESS_INFORMATION pi;
   PFString            sCmdLine;

//   ASSERT( NULL != pszCmd );

   if( NULL == pszCmd )
   {
      return( false );
   }

   if( NULL == pszCmdLine )
   {
      sCmdLine = pszCmd;
   }
   else
   {
     sCmdLine = pszCmd;
     sCmdLine += " ";
     sCmdLine += pszCmdLine;
   }

   // initialize the STARTUPINFO stucture
   ::ZeroMemory( &si, sizeof( STARTUPINFO ) );
   si.cb = sizeof( STARTUPINFO );

   // create a new process
   if( ::CreateProcess( NULL,                               // pointer to name of executable module
                        ( LPTSTR ) ( LPCTSTR ) sCmdLine,    // pointer to command line string
                        NULL,                               // ptr to process attributes
                        NULL,                               // ptr to thread attributes
                        FALSE,                              // inherit handles
                        NORMAL_PRIORITY_CLASS,              // creation flags
                        NULL,                               // ptr to new environment block

                        //pszCurrentDirectory,                // ptr to drive\dir for process
                  NULL,

                        &si,                                // ptr to startup info struct
                        &pi ) )                             // ptr to process info struct
   {
      ::CloseHandle( pi.hThread );

      if( TRUE == bWait )
      {
         ::WaitForSingleObject( pi.hProcess, dwMilliseconds );
      }

      ::CloseHandle( pi.hProcess );
   }
   else
      return false;

   return true;
}  // End of Function: LaunchApp( )
#endif

bool PFScriptFile::Shell(char *args) {
   IPFSymbol *pSymbol=m_pTable->LookupSymbol(args);
   if (pSymbol==NULL) {
      PFOutput::EnableOneLineForceScreenOutput();
      PFPrintfStderr("Script-Error: Unknown variable name on line %d\n",m_nInstrPtr+1);
      return false;
   }

   PFString tStr=pSymbol->GetStringValue();

   int ret;
#if defined (_MSC_VER)
   // Try doing this WITHOUT a command window, if this is a GUI app (aka ECM-4c-r5-gui.exe build)
   char *Buf = new char[tStr.GetLength()+1];
   char App[1024];
   strcpy(Buf, tStr);
   char *cp = strchr(Buf, ' ');
   if (cp)
   {
      *cp++ = 0;
      _fullpath(App, Buf, sizeof(App));
      //PFPrintfLog("LaunchApp: %s\n", tStr);
      ret=LaunchApp(App, cp, TRUE, INFINITE);
   }
   else
   {
      //PFPrintfLog ("system: %s\n", tStr);
      ret=system(tStr);
   }
#else
   ret=system(tStr);
#endif


   pSymbol=m_pTable->LookupSymbol("ERRORLEVEL");
   if (pSymbol==NULL || pSymbol->GetSymbolType()!=INTEGER_SYMBOL_TYPE) {
      PFOutput::EnableOneLineForceScreenOutput();
      PFPrintfStderr("Script-Error: Internal error, ERRORLEVEL not set correctly.");
   } else {
      PFIntegerSymbol *pVar=(PFIntegerSymbol *)pSymbol;
      Integer *iVal=new Integer(ret);
      iVal=pVar->SetValue(iVal);
      delete iVal;
   }

   return true;
}

