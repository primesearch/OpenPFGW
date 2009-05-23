
#if !defined (INI_H_)
#define INI_H_

class IniFile
{
  public:
    IniFile(const char *IniFileName);
    ~IniFile();

    bool IniGetInt    (const char *Key, const char* SubKey, int *RetValue,  int Default);
    bool IniGetUInt   (const char *Key, const char* SubKey, unsigned *RetValue,  int Default);
    bool IniGetDWORD  (const char *Key, const char* SubKey, DWORD *RetValue,  DWORD Default); // uses base 16
    bool IniGetString (const char *Key, const char* SubKey, CString *RetValue, const char* Default);
    bool IniGetBoolI  (const char *Key, const char* SubKey, int *RetValue, bool Default);
    bool IniGetBool   (const char *Key, const char* SubKey, bool *RetValue, bool Default);

    bool IniSetDWORD  (const char *Key, const char* SubKey, DWORD ToValue);  // Reads from base 16
    bool IniSetInt    (const char *Key, const char* SubKey, int ToValue);
    bool IniSetUInt   (const char *Key, const char* SubKey, unsigned ToValue);
    bool IniSetBool   (const char *Key, const char* SubKey, bool ToValue);
    bool IniSetString (const char *Key, const char* SubKey, const char *ToValue);

	// Forces a flush under Win95.  Call when done updating.
	void IniFile::Flush  ();

	// See if a section already exists in the .ini file
	bool IniFindSection(const char *Key);

    bool IsValid()    { return m_Valid;}

	const char *IniName() { return m_FName; }

	void AddDefaultSection(const char *SectionName, const char *MonitorName="");
	void LoadWhatToDo(const char *WhichSection);


  private:
    char m_FName[260];
    bool m_Valid;
	bool m_bIsWin95;

};

#endif
