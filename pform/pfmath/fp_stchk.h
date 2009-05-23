// a simle inline function used to test whether the FP stack is empty.
// It will only compile in code in debug build (under VC), so sprinkling
// calls to DEBUG_ADJUST_FP_STACK() is "safe" speed-wise. For a release
// build, nothing will be compiled in.

#if !defined (___fp_stchk__h_)
#define ___fp_stchk__h_

#if defined (_MSC_VER)
	#if defined (_DEBUG)
		#pragma pack(1)
		struct fp_Data
		{
			// This structure is only valid in 32 bit protected mode of the CPU
			unsigned short CtlW;
			unsigned short Reserve1;
			unsigned short StatW;
			unsigned short Reserve2;
			unsigned short TagW;
			unsigned short Reserve3;
			unsigned int   FPUInstrPtrOffset;
			unsigned short FPUInstrSelector;
			unsigned short FPUInstr; // high 4 bits are always 0,0,0,0
			unsigned int   FPUOpPtrOffset;
			unsigned short FPUOpSelector;
			unsigned short Reserve4;
		};
		#pragma pack()

		__inline void DEBUG_ADJUST_FP_STACK(char *File, int Line)
		{
			DoStackAdjustAgain:;
			static fp_Data ffdat;
			__asm push eax;
			__asm lea eax, ffdat;
			__asm fnstenv dword ptr [eax];
			__asm pop eax;
			if (ffdat.TagW != 0xFFFF)
			{
				fprintf (stderr, "** Bad FPU stk %s %d\n", File, Line);
				__asm fstp st(0);
				goto DoStackAdjustAgain;
			}
		}
	#else
		// in debug mode VC, we don't have this check.
		#define DEBUG_ADJUST_FP_STACK(a,b)
	#endif // defined _DEBUG
#else
	// Don't know how to deal with this anywhere other than VC (and only deal with it in VC in debug mode
	#define DEBUG_ADJUST_FP_STACK(a,b)
#endif

#endif // #if !defined (___fp_stchk__h_)
