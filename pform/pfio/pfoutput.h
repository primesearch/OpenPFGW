// pfoutput.h
//
//   Contains global functions PFPrintfStderr(const char *Fmt, ...) and PFPrintfLog(const char *Fmt, ...)
//
//   Also will contain a "class" which will help go between all GUI/console apps.

#if !defined (__PFOutput_H__)
#define __PFOutput_H__

#include <stdarg.h>
#include <stdio.h>

#ifdef _MSC_VER
#define access _access
#endif

#include "../../pform/pflib/pfstring.h"

extern "C" {

// %79.79s is "almost" full line.  This will clear the current line "almost" every time, and will NOT cause a 
// line feed ever (unless less than 80 columns!)
void PFPrintfClearCurLine(int line_len=79); 
int PFfflush(FILE *); 
void PFWriteErrorToLog(const char *expr, const char *msg1, const char *msg2, const char *msg3, const char *msg4=0);

int PFPrintfStderr(const char *Fmt, ...);
int PFPrintf(const char *Fmt, ...);
int PFPrintfLog(const char *Fmt, ...);
int PFPrintfLogOnly(const char *Fmt, ...);

};

class PFOutput
{
	public:
		PFOutput();
		virtual ~PFOutput();

		virtual void PFPrintfClearCurLine(int line_len)=0;
		virtual int PFfflush(FILE *) = 0;

		void InitLogFile(const char *FName, const int terseOutput);
		void CloseLogFile();
		int  PFLogPrintf (const char *Fmt, va_list &va);

		// Allows an app to "shut off" IO.  Mostly for GUI apps that don't care about the IO.
		// In the GUI version, a buffer is allocated and a PostMessage sends the GUI that 
		// string.  The GUI's message pump MUST delete that buffer, or a memory leak will
		// occur.  If the GUI is not interested in processing the message, it STILL must 
		// handle the message, just to delete the pointer (see PFGW_ServerV2).  Now, by
		// simply creating the GUI object, and calling DisableAllScreenOutput(), then
		// there will be no output generated.

		// ALSO, this might be a GOOD way to cut down on the screen noise in PFGW.  Simply
		// finding the "right" places to put the EnableAllScreenOutput() and the 
		// DisableAllScreenOutput() functions, if the user wants "minimal" output,
		// or simply placing a DisableAllScreenOutput() if the user want no output
		// at all.  Currently, redirection of stdout and stderr are required to "quiet"
		// PFGW.

		static void EnableOneLineForceScreenOutput() {m_bForcePrint=true;}

      static void PFWriteErrorToLog(const char *expr, const char *msg1, const char *msg2, const char *msg3, const char *msg4);

		virtual int PFPrintfStderr(const char *Fmt, va_list &va)=0;
		virtual int PFPrintf      (const char *Fmt, va_list &va)=0;

	protected:
		static bool m_bForcePrint;
      char *m_pBuffer;
      long  m_iBufferSize;

	private:
		char *m_OutputLogFileName;
		FILE *m_fpOutputLog;

		// effective C++ requires these overrides.
		// They are declared, but not actually defined here		
		PFOutput(const PFOutput &);
		PFOutput &operator=(const PFOutput &);
};

class PFConsoleOutput : public PFOutput
{
	public:
		PFConsoleOutput();
		~PFConsoleOutput();

		void PFPrintfClearCurLine(int line_len);
		int PFfflush(FILE *);
		int PFPrintfStderr(const char *Fmt, va_list &va);
		int PFPrintf      (const char *Fmt, va_list &va);
};

class PFWin32GUIOutput : public PFOutput
{
	public:
		PFWin32GUIOutput(long long hWnd);
		~PFWin32GUIOutput();

		void PFPrintfClearCurLine(int line_len);
		int PFfflush(FILE *);
		int PFPrintfStderr(const char *Fmt, va_list &va);
		int PFPrintf      (const char *Fmt, va_list &va);

	private:
      long long m_hWnd;	// actually an HWND, but I want this to "compile" easily :)
};


// This is the "global" output device.  It can be either a console type or a Win32GUI type at this time.
// One of the FIRST thins an app needs to do is to allocate this object to the "correct" type class for the app.
extern PFOutput *pOutputObj;

#endif   // __PFOutput_H__
