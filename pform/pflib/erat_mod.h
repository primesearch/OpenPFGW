/*-------------------------------------------------------------------------
/                                                                         *
/ Erat_Mod.h    Copyright Jim Fougeron, 2000/2001.  All rights reseved.   *
/                                                                         *
/ Usage of this code by other developers is granted by the author, Jim    *
/ Fougeron, however, the author does require written notification before  *
/ this code is put into a project.  This copyright notice MUST be         *
/ maintained in its original form within the source code.  Also, if       *
/ the functionality of this code becomes a significant part of the        *
/ finished product/utility, then the author requires a cursory            *
/ acknowledgement in the program's "about" or "startup" text screen.      *
/                                                                         *
/ The author also reqests that all modifications to the code, and         *
/ bug fixes or porting issues be sent to him to incorporate into          *
/ on going versions of this code.                                         *
/                                                                         *
/-------------------------------------------------------------------------*/

// Description:
//
// This file implements a class around the modular sieve of Eratosthenes 
// implemented by Jim Fougeron.
//

#if !defined (ERAT_MOD_H___)
#define ERAT_MOD_H___

class Erat_Mod
{
#if defined (TESTING)
	friend int main(int, char**);
#endif
	public:
		Erat_Mod(const char* StartupString);
		Erat_Mod(uint32 ModularSkip, bool bIsModPlus1=true, uint32 Start_SIEVE_BIT_SIZE=16, uint32 Continuing_SIEVE_BIT_SIZE=20);
		~Erat_Mod();

		void skipto(uint64 skip);
		void init();
		uint64 next();
		void  AddModCondition(int Mod, int Val, bool bOnlyAcceptIfTrue=true);
		void  AddSmallPrimesExceptions(uint32 nMaxSmallPrime);
		void  AddFactorsExceptions(uint64 nNum);
		//peek();			// Not implemented, because it is not used.  
		//count();			// ''

		void SetSieveBitMapSize(uint32);

		// Call this function before program shutdown.  It will clean up any memory for static data, or for the caches.
		static void FreeAllMemory();
		bool	isValid();
		uint32	GetModVal() { return  m_nModBase; }

	private:
		void Erat_Mod_init(uint32 ModBase, bool bIsModPlus1, uint32 nSieveBitSize, uint32 Continuing_SIEVE_BIT_SIZE);
		uint32 modInv(const uint32 x, const uint32 m);
		void FillSmallPrimes(uint32 Nsp);
		void FillSmall_ixt();
		void SModLoadNextSieveChunk();
		bool AdjustDepth();

		struct ModCondition
		{
			int Mod;
			int Val;
			bool bOnlyAcceptIfTrue;
		};
		ModCondition	*m_ModConditions;
		uint32	m_nModConditions;

		uint32	*m_SModpMap, *m_SMod_pMap;
		uint32	m_nModBase;
		bool	m_bIsModPlus1;
		bool	m_bvalid;
		bool	m_bAdjusted;

		uint32	m_n_spTabnCur;
		uint32	m_SIEVE_BIT_SIZE;
		uint32	m_ContinuingSIEVE_BIT_SIZE;

		// primetable[] is an array of small primes from 3 to whatever the max prime we are using is, 
		static uint32 *primetable;
		static uint32  maxSmallPrime;
		// ixt[] is the "starting place" for that prime within the next chunk to fill.
		uint64	*m_ixt;
		uint64	*m_Exceptions;
		uint32	m_nCurException;
		uint32	m_nNumExceptions;
		bool	m_bSModLoadNextSieveChunk_Adjusted;

		uint64 m_uiNext;
		uint64 m_uiLast;
		uint64 m_SModMaxNum;
	private:
		Erat_Mod(const Erat_Mod &);
		Erat_Mod& operator=(const Erat_Mod &);		
};


#endif  // ERAT_MOD_H___

