// PFABCTaskCompleted.h
//
//  This class is in "charge" of working with completed tasks within
//  ABC* type files.  If the comment {number_primes,$?,#} or {number_comps,$?,#} is
//  seen, then the ABC processing must stop with # primes (or comps) is seen for the
//  $? variable.  Then all of this same values for that variable are NOT processed from
//  that point in the file forward.

//  NOTE save/resume DOES NOT work in this mode.

struct Complete_t
{
	uint64_t Value;
	uint32_t NumDone; // incremented by prime or composite.
};

class PFABCTaskCompleted
{
	public:
		PFABCTaskCompleted(const char *pExpr, const char *pABC);
		~PFABCTaskCompleted();

		bool ProcessThisValue(char *s_array[26]);
		void AddPrimeOrComposite(char *s_array[26], bool bIsPrime);

		bool bProcessingForPrimes() { return m_bPrimes; }
		int  nHowMany()             { return m_NumDones; }

	private:
		// This variable are used with the ProcessThisLine() function (and the {number_primes,$b,1}
		uint32_t m_WhichDone;			// This is 0 for $a 1 for $b, ... 24 for $z
		uint32_t m_NumDones;			// how MANY primes(composites) need to be found before breaking?
		uint64_t *m_DoneList;			// This is a "sorted" array of uint64_t's (which can be bsearched quickly
		uint32_t m_nDoneCnt;			// This is the number located in m_DoneList.
		uint64_t m_DoneListTemp[16];	// A temp unsorted list.  When this fills, we add these to m_DoneList, and resort m_DoneList.  This temp list is simply to keep from doing too much sorting.
		uint32_t m_nDoneTempCnt;		// A count of the "temp" list.
		bool   m_bPrimes;

		Complete_t  *m_pCompleted;	// Sorted set of "how many have been found" list.  This is used to add values TO the m_DoneList (and m_DoneListTemp) lists.
		Complete_t  m_CompletedTemp[16];	// A temp unsorted list, to avoid sorting.
		uint32_t m_nCompleted;
		uint32_t m_nCompletedTemp;
      
      bool ProcessThisValue(uint64_t x);
      void AddPrimeOrComposite(uint64_t x);

      // When a value is completed, it is "added" to the list of dones (this is only done once).  
		// Note it is NOT removed from the m_pCompleted list, but "could" be (that would cause a resort),
		// but we might be able to do that if we set the Value to a high number (like 0xFFFFFFFFFFFFFFFF, 
		// then it will sort to the top, and we could reduce the m_nCompleted number by the number of these
		// that bubble to the top.  That has not yet been done, but could be.
		void AddDone(uint64_t Value);
};
