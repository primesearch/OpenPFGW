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

#include <inttypes.h>

#if !defined (ERAT_MOD_H___)
#define ERAT_MOD_H___

class Erat_Mod
{
#if defined (TESTING)
	friend int main(int, char**);
#endif
	public:
		Erat_Mod(const char* StartupString);
		Erat_Mod(uint32_t ModularSkip, bool bIsModPlus1=true, uint32_t Start_SIEVE_BIT_SIZE=16, uint32_t Continuing_SIEVE_BIT_SIZE=20);
		~Erat_Mod();

		void skipto(uint64_t skip);
		void init();
		uint64_t next();
		void  AddModCondition(int Mod, int Val, bool bOnlyAcceptIfTrue=true);
		void  AddSmallPrimesExceptions(uint32_t nMaxSmallPrime);
		void  AddFactorsExceptions(uint64_t nNum);
		//peek();			// Not implemented, because it is not used.  
		//count();			// ''

		void SetSieveBitMapSize(uint32_t);

		// Call this function before program shutdown.  It will clean up any memory for static data, or for the caches.
		static void FreeAllMemory();
		bool	isValid();
		uint32_t	GetModVal() { return  m_nModBase; }

	private:
		void Erat_Mod_init(uint32_t ModBase, bool bIsModPlus1, uint32_t nSieveBitSize, uint32_t Continuing_SIEVE_BIT_SIZE);
		uint32_t modInv(const uint32_t x, const uint32_t m);
		void FillSmallPrimes(uint32_t Nsp);
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
		uint32_t	m_nModConditions;

		uint32_t	*m_SModpMap, *m_SMod_pMap;
		uint32_t	m_nModBase;
		bool	m_bIsModPlus1;
		bool	m_bvalid;
		bool	m_bAdjusted;

		uint32_t	m_n_spTabnCur;
		uint32_t	m_SIEVE_BIT_SIZE;
		uint32_t	m_ContinuingSIEVE_BIT_SIZE;

		// primetable[] is an array of small primes from 3 to whatever the max prime we are using is, 
		static uint32_t *primetable;
		static uint32_t  maxSmallPrime;
		// ixt[] is the "starting place" for that prime within the next chunk to fill.
		uint64_t	*m_ixt;
		uint64_t	*m_Exceptions;
		uint32_t	m_nCurException;
		uint32_t	m_nNumExceptions;
		bool	m_bSModLoadNextSieveChunk_Adjusted;

		uint64_t m_uiNext;
		uint64_t m_uiLast;
		uint64_t m_SModMaxNum;
	private:
		Erat_Mod(const Erat_Mod &);
		Erat_Mod& operator=(const Erat_Mod &);		
};


#endif  // ERAT_MOD_H___

