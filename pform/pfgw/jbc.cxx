
#define JBC()                                                                \
	DWORD iComma=sPhi.Find(",");                                             \
	if(iComma!=NOT_FOUND)                                                    \
	{                                                                        \
		PFString s1=sPhi.Left(iComma);                                       \
		PFString s2=sPhi.Mid(iComma+1);                                      \
		/* we need iPhi  */                                                  \
		sprintf(sExpression,"p(%s)^(%s+1)",LPCTSTR(s2),LPCTSTR(s1));         \
		Integer *P=ex_evaluate(psym,sExpression);                            \
		if(P)                                                                \
		{                                                                    \
			iPhi=(*P)&0x7FFFFFFF;                                            \
			if((*P)!=iPhi)                                                   \
			{                                                                \
				iPhi=0;                                                      \
			}                                                                \
		}                                                                    \
		sprintf(sExpression,"Phi(p(%s)^(%s+1),2)",LPCTSTR(s2),LPCTSTR(s1));  \
		sprintf(g_cpTestString,"F[%s,%s]",LPCTSTR(s1),LPCTSTR(s2));                \
	}                                                                        \
	else                                             

