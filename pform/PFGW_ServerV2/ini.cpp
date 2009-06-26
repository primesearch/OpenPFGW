#include "stdafx.h"
#include <io.h>

#include "ini.h"

#if _MSC_VER
#define access _access
#endif

// class IniFile
IniFile::IniFile(const char *IniFileName)
{
	_fullpath(m_FName, IniFileName, sizeof(m_FName)-1);
	m_Valid = false;
	if (!access(m_FName, 0))
	{
		m_Valid = true;
		m_bIsWin95 = (GetVersion() > 0x80000000);
	}
}

IniFile::~IniFile()
{
}

bool IniFile::IniGetInt   (const char *Key, const char* SubKey, int *RetValue,  int Default)
{
   if (!m_Valid)
   {
      *RetValue = Default;
      return false;
   }
   *RetValue = GetPrivateProfileInt(Key, SubKey, Default, m_FName);
   return true;
}
bool IniFile::IniGetUInt   (const char *Key, const char* SubKey, unsigned *RetValue,  int Default)
{
    return IniGetInt(Key, SubKey, (int*)RetValue, Default);
}
bool IniFile::IniGetString(const char *Key, const char* SubKey, CString *RetValue, const char* Default)
{
   if (!m_Valid)
   {
      *RetValue = Default;
      return false;
   }
   char Buf[256];
   GetPrivateProfileString(Key, SubKey, Default, Buf, sizeof(Buf), m_FName);
   *RetValue = Buf;
   return true;
}
bool IniFile::IniGetBool  (const char *Key, const char* SubKey, bool *RetValue, bool Default)
{
   if (!m_Valid)
   {
      *RetValue = Default;
      return false;
   }
   char UBuf[80];
   char *Def = "false";
   if (Default)
      Def = "true";
   GetPrivateProfileString(Key, SubKey, Def, UBuf, sizeof(UBuf), m_FName);
   if (*UBuf == 0)
      *RetValue = Default;
   else
   {
      *RetValue = false;
      if (!strcmp(UBuf, "1"))
         *RetValue = true;
      if (!_stricmp(UBuf, "true"))
         *RetValue = true;
   }
   return true;
}

bool IniFile::IniGetBoolI (const char *Key, const char* SubKey, int *RetValue, bool Default)
{
   *RetValue = 0;
   return (IniGetBool(Key, SubKey, (bool*)RetValue, Default));
}

bool IniFile::IniSetString(const char *Key, const char* SubKey, const char *ToValue)
{
   int Val = WritePrivateProfileString(Key, SubKey, ToValue, m_FName);
   if (!Val)
      Val = GetLastError();
   return true;
   
}

bool IniFile::IniSetInt   (const char *Key, const char* SubKey, int ToValue)
{
   char Buf[80];
   sprintf (Buf, "%d", ToValue);
   return IniSetString(Key, SubKey, Buf);
}

bool IniFile::IniSetUInt   (const char *Key, const char* SubKey, unsigned ToValue)
{
   char Buf[80];
   sprintf (Buf, "%u", ToValue);
   return IniSetString(Key, SubKey, Buf);
}

bool IniFile::IniSetBool  (const char *Key, const char* SubKey, bool ToValue)
{
   if (!ToValue)
      return IniSetString(Key, SubKey, "false");
   return IniSetString(Key, SubKey, "true");
}

// Reads from base 16
bool IniFile::IniSetDWORD  (const char *Key, const char* SubKey, DWORD ToValue)
{
   char Buf[80];
   sprintf (Buf, "%08X", ToValue);
   return IniSetString(Key, SubKey, Buf);
}

// uses base 16
bool IniFile::IniGetDWORD  (const char *Key, const char* SubKey, DWORD *RetValue,  DWORD Default)
{
   if (!m_Valid)
   {
      *RetValue = Default;
      return false;
   }
   char UBuf[80], Def[16];
   sprintf(Def, "%08X", Default);
   *UBuf = 0;
   GetPrivateProfileString(Key, SubKey, Def, UBuf, sizeof(UBuf), m_FName);
   if (*UBuf == 0)
      *RetValue = Default;
   else
		*RetValue = strtol(UBuf, 0, 16);
   return true;

}

bool IniFile::IniFindSection(const char *Key)
{
	char SectionNames[4096];

	DWORD Len = GetPrivateProfileSectionNames(SectionNames, sizeof(SectionNames), m_FName);
	if (!Len)
		return false;

	char *cp = SectionNames;
	while (*cp)
	{
		if (!_stricmp(cp, Key))
			return true;
		cp += strlen(cp);
		cp++;
	}
	return false;
}


void IniFile::Flush  ()
{
   WritePrivateProfileString(0, 0, 0, m_FName);
}


