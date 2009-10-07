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

