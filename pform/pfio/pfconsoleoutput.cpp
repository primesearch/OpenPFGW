// PFConsoleOutput.cpp
//
//   Contains global functions PFPrintfStderr(const char *Fmt, ...) and PFPrintf(const char *Fmt, ...)
//
//   Also will contain a "class" which will help go between all GUI/console apps.



#include "pfiopch.h"
#include <stdio.h>
#include <string.h>

PFConsoleOutput::PFConsoleOutput() : PFOutput()
{
}

PFConsoleOutput::~PFConsoleOutput()
{
}

int PFConsoleOutput::PFPrintfStderr(const char *Fmt, const va_list &va)
{
	int x = vfprintf(stderr, Fmt, va);
	fflush(stderr);

	if (m_bErrorPrint)
	{
		m_bErrorPrint = false;  // clear so the next print does not print this.
		FILE *out = fopen("pfgw_err.log", "a");
		if (out)
		{
			time_t t = time(NULL);
			fprintf(out, "-----------------------------------------------------------------------\n");
			fprintf(out, "Error occuring in PFGW at %s", ctime(&t));
			fprintf(out, "Expr = %s\n", (const char*)m_ErrExpr);
			fprintf(out, "Failed at bit %d of %d\n", m_BitsDone, m_BitsTotal);
			if (m_ErrMsg)
				fprintf(out, "Msg = %s\n", (const char*)m_ErrMsg);
			vfprintf(out, Fmt, va);
			fprintf(out, "\n");
			fclose(out);
		}
	}
	return x;
}

int PFConsoleOutput::PFPrintf(const char *Fmt, const va_list &va)
{
	int x = vprintf(Fmt, va);
	fflush(stdout);
	return x;
}

void PFConsoleOutput::PFPrintfClearCurLine(int line_len)
{
	if (line_len < 1)
		line_len = 1;
	else if (line_len > 79)
		line_len = 79;
	fprintf(stderr, "\r%*.*s\r", line_len, line_len, " ");
}

int PFConsoleOutput::PFfflush(FILE *f)
{
	return fflush(f);
}

