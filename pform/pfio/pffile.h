//  PFSimpleFile class for file handling

#if !defined (__PFFile_H__)
#define __PFFile_H__

#if !defined (_WIN_COPY_ONLY_)
#include "pflib.h"
#include "pfmath.h"
#endif

class PFIni;


class PFSimpleFile
{
	public:
		PFSimpleFile(const char* FileName);
		int SecondStageConstruction(PFIni* pIniFile);

		// return values
		enum {e_eof=-1, e_ok=0, e_bad_file, e_unknown};

		// Virtual functions

		virtual ~PFSimpleFile();

		virtual int GetNextLine(PFString &sLine, Integer *i=0, bool *b=0);
		virtual int GetCurrentLineNumbers(int &nVirtualLineNumber, int &nPhysicalLineNumber);
		virtual int Rewind();
		virtual int SeekToLine(int LineNumber);

		// These functions are not used in the PFSimpleFile class, they return e_unknown, however
		// they are used in some of the derived classes, and are declared here as virtual, so that
		// a PFSimpleFile *v pointer can be used and ALL possible functions will work fine (even if
		// they do nothing in this base class).
		virtual int GetKNB(uint64 &k, uint64 &n, unsigned &b);
		virtual void CurrentNumberIsPrime(bool bIsPrime, bool *p_bMessageStringIsValid=0, PFString *p_MessageString=0);

	protected:
		// in the base class, this is a no-op, but a derived class should PFPrintf who it is.
		virtual void Printf_WhoAmIsString() {}
		// this signature will be used as another "validation" that the file we are restarting from is the "correct" file.
		// This data will get stored into the .ini file, and then compared against the ini file during a restart. ALL
		// derived classes should override this function.
		virtual PFString SignatureString() { return "SIMPLE_FILE"; }
		// Again, a no-op.  Most derived classes will overload this, since most of them have special handling for
		// the first line.  This function will be called by SecondStageConstruction()
		virtual void LoadFirstLine() {}
		// by default a PFFile will process a line.  This "can" be over-ridden to allow a derived class
		// to stop processing "certain" lines in a file (after some "condition" has been met, for instance).
		virtual bool ProcessThisLine();

	protected:
		// default constructor needed for PFStringFile, however, you don't want this method available for outside user. It 
		// has been made protected to "hide" it's interface from the outside.  NOTE when this constructor is called, the
		// FILE pointer is null, so ALL functions which access the m_fpInputFile must be re-defined. PFStringFile does this
		PFSimpleFile();
		int m_nCurrentLineNum;
		int m_nCurrentPhysicalLineNum;
		FILE *m_fpInputFile;
		char *m_cpFileName;
		PFIni *m_pIni;

		// Used to "store" the current line's expression.  this value is stored into the ini file upon destruction
		// ALL derived classes should use this "variable" to build expressions into
		PFString m_sCurrentExpression;

		// Fill out this string to be used in the Printf_WhoAmIsString() function.
		PFString m_SigString;

		bool m_bEOF;
		// Need to be virtual. Then ABCD file simply changes how this function works, and ABC processing
		// will work without change.
		virtual int ReadLine(char *Line, int sizeofLine);
		
	private:
		// effective C++ requires these overrides.
		// They are declared, but not actually defined here		
		PFSimpleFile(const PFSimpleFile &);
		PFSimpleFile &operator=(const PFSimpleFile &);
};


//
//
//   PFStringFile   
// 
//   This class is based upon the PFSimpleFile, and adds ability to handle command line entered expression, and can
//   possibly be "morphed" into a "interpreter" similar to the *nix "bc" little language interpreter
//
//

class PFStringFile : public PFSimpleFile
{
	public:
		PFStringFile();
		~PFStringFile();

		int GetNextLine(PFString &sLine, Integer *i=0, bool *b=0);
		int Rewind();
		int SeekToLine(int LineNumber);

		int WriteToString(const char *String);  // these are a "uniq" function to this class.
		int AppendString(const char *String);
		int ClearString(const char *String);

		void Printf_WhoAmIsString() {}
		// kind of silly, since a string file is only one expression can't restart, but it is here for semantics
		PFString SignatureString() { return "STRING_FILE"; }


	private:
		PFString sData;
		bool bUsed;
};

// Global function to "allocate" an unknown file.  This function will allocate a PFSimpleFile or a PFNewPGenFile
// depending upon if the file is a NewPGen log file, or simply a file containing expressions.
PFSimpleFile *openInputFile(const char *FileName, PFIni* pIniFile=0, const char **ErrorMessage=0);

#endif
