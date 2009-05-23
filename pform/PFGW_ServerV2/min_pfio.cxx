
/*
inline int PFPrintfStderr(const char *Fmt, ...)
{
#if defined (_DEBUG)
	char Buf[8192];
	va_list va;
	va_start (va, Fmt);
	vsprintf(Buf, Fmt, va);
	va_end(va);
	MessageBox(0, "Error", Buf, 0);
#endif
	return 0;
}

inline int PFPrintf(const char *Fmt, ...)
{
	return 0;
}
*/
