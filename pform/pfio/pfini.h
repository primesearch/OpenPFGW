// pfni.h
//
//  This class allows reading/writing of "ini-type" files.  I think that the INI format is
//  unique to Win32, so I am not using GetPrivateProfileString() WritePrivateProfileString()
//  functions, but have "made my own".  These function provide "caching", and keep lines
//  in order.

#if !defined (__PF_INI_H_)
#define __PF_INI_H_

#include <stdint.h>
#include <time.h>   // for time_t
#if defined (_MSC_VER)
#include <windows.h>
#endif

class PFIniEntry
{
    public:
      PFIniEntry(const PFString *pSection=0, const PFString *pKey=0,const PFString *pData=0);
      ~PFIniEntry();
      // returns "current" Key, and possibly sets the Key (if pNewKey is not null)
      const PFString &Key(const PFString *pNewKey=0);
      // returns "current" Data, and possibly sets the Data (if pNewData  is not null)
      const PFString &Data (const PFString *pNewData=0);
      // returns "current" Section, and possibly sets the Section (if pNewSection  is not null)
      const PFString &Section (const PFString *pNewSection=0);
      // returns "current" Line in total
      const PFString &Line (const PFString *pNewLine=0);

   private:
      PFString *m_pSection;
      PFString *m_pKey;
      PFString *m_pData;
      PFString *m_pLine;

      // effective C++ requires these overrides.
      // They are declared, but not actually defined here
      PFIniEntry(const PFIniEntry&);
      PFIniEntry& operator=(const PFIniEntry&);
};

class PFIni
{
   public:
      PFIni(const char* FileName="pfgw.ini");
      ~PFIni();

      enum PFIniErr {eOK=0,
                    eFileNotFound,
                  eIniNotOpened,
                  eSectionNotFound,
                  eSectionNotSet,
                  eEntryNotFound,
                  eInvalidEntryTypeRequest};

      PFIniErr SetCurrentSection(const char *SectionName);
      void GetCurrentSection(PFString &s);

      PFIniErr GetIniString(PFString *pOutStr,  PFString *pKeyName, PFString *pDefault=NULL,  bool bAddIfNotExist=false);
      PFIniErr GetIniInt   (int      *pOutInt,  PFString *pKeyName, int       nDefault=0,     bool bAddIfNotExist=false);
      PFIniErr GetIniBool  (bool     *pOutBool, PFString *pKeyName, bool      bDefault=false, bool bAddIfNotExist=false);

      PFIniErr SetIniString(PFString *pSetTo, PFString *pKeyName);
      PFIniErr SetIniInt   (int       nSetTo, PFString *pKeyName);
      PFIniErr SetIniBool  (bool      bSetTo, PFString *pKeyName);

      PFIniErr DeleteIniKey(PFString *pKeyName);
      PFIniErr DeleteIniSection(PFString *pSectionName);

      void ForceFlush();

      // File manipulation functions.
      void IncFileLineNum();
      void SetFileLineNum(int LineNum=0);
      int  GetFileLineNum();
      void SetFileName(PFString *pFName);
      void SetOutputFileName(PFString *pFName);
      PFString *GetFileName();                  // Allocated PFString.  Caller MUST delete the pointer when done.
      PFString *GetOutputFileName();               // Allocated PFString.  Caller MUST delete the pointer when done.
      void SetFileProcessing(bool bProcessing=true);  // set to false when the file is completely done.
      bool GetFileProcessing();                 // if false, then there was no file processing.
      void SetExprChecksum(PFString *pCurrentExpression);
      bool CompareExprChecksum(PFString *pCurrentExpression, PFString *pStoredExpression);
      bool IsExprChecksumNull();
      void GetDefaultSettings(PFString *pDefault);

      // store off the address of this.  The value of this will not be sent every line, but the ForceFlush can "get" the
      // data when it needs to.  I don't want to pass a PFString across (several copy constructors plus lots of other
      // excess processing needed) for every line of the file.  This would simply be too much overhead for small number
      // searches (1000 digit range).  When we need to "flush" the ini file, now we can keep the line count and the
      // 'current' expression stuff in sync.  Now if the program locks up, or the power gets turned off, the file
      // restart will still be valid.
      void AssignPointerOfCurrentExpression(PFString *p);

      PFString sfalse;
      PFString strue;

   protected:
      // Cache writing, and only write if more than 1 minute has elapsed.  NOTE that ForceFlush will write EVERY time,
      // and this can be called when needed (like program exit, or complete proceessing of a file).  ForceFlush will
      // update this variable to the current time.  The constructor also uses it.
      char *m_szCurrentSection;
      char *m_szCurrentFileReadSection;
      int  m_nCurFileLineNum, m_nLastFileLineNum;
      DWORD m_dwSectionStart, m_dwSectionEnd;

      static time_t tt_lastFlushTime;
      static char *m_szFileName;
      static int m_nRefCount;
      static PFPtrArray<PFIniEntry> *m_pDataArray;
      static PFString *m_pCurrentFileLineExpressionString;

      // private functions are ALL prepended by a _ char.  They are not syncronized.  They will only be
      // called by "public" functions which are already fully syncronized.
      FILE *_CreateIniFile();
      int _GetNextLine(FILE *fp, PFIniEntry *p);

      PFIniErr _SetCurrentSection(const char *SectionName);
      void _GetCurrentSection(PFString &s);

      PFIniErr _GetIniString(PFString *pOutStr,  PFString *pKeyName, PFString *pDefault=NULL,  bool bAddIfNotExist=false);
      PFIniErr _GetIniInt   (int      *pOutInt,  PFString *pKeyName, int       nDefault=0,     bool bAddIfNotExist=false);
      PFIniErr _GetIniBool  (bool     *pOutBool, PFString *pKeyName, bool      bDefault=false, bool bAddIfNotExist=false);

      PFIniErr _SetIniString(PFString *pSetTo, PFString *pKeyName);
      PFIniErr _SetIniInt   (int       nSetTo, PFString *pKeyName);
      PFIniErr _SetIniBool  (bool      bSetTo, PFString *pKeyName);

      void _SetExprChecksum(PFString *pCurrentExpression);

      // Sycronization functions.  This allows multi-threaded useage.  NOTE that in a
      // single threaded build (PFGW console), these functions should be inlined NO-OP functions.
      void CS_Lock();
      void CS_Release();
      void _ForceFlush();
      void CS_Init();
      void CS_Free();
#if defined (_MSC_VER)
      static CRITICAL_SECTION m_sCS;
      static LONG m_CntCS;
#endif

      // this will only call "ForceFlush" if the "time-out" has expired (currently 1 minute).
      void _FlushIfTimedOut();
      bool m_bDirty;

      // If we can not open or create the pfgw.ini file, then we are NOT valid to write data.
      // We allow pfgw to continue processing (with no restart capability), but this object
      // MUST handle this condition by "gracefully" failing every time it is called.
      bool m_bValid;
      bool m_bProcessing;

   private:
      // effective C++ requires these overrides.
      // They are declared, but not actually defined here
      PFIni(const PFIni&);
      PFIni &operator=(const PFIni&);

};

extern PFIni *g_pIni;

// Inlines (mostly to keep them out of pfini.cpp for now)
#include "pfini.inl"

#endif // __PF_INI_H_
