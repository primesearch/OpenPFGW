// class TreeFactorize
//
// This class was inspired by Jens Kruse Andersen's TreeSieve program
//
// Before ever attempting to put this code into PFGW, I wrote these 
// classes and inserted them into Jens TreeSieve.  There was a command
// line switch, and either Jens' code, or my code (JimF) could be run.
// I did this to get things working correctly, and it was also used to
// test out different optimizations.

// For now, we use the "raw" GMP types, and not Integer.  Later we may see
// how much the Integer class slows this down.  If not much, we certainly
// will use that, due to the fact the Integer class hides the underlying
// gmp (which is we switch to some other large integer lib, if we use Integer
// class, such a change would be transparent).  For now, however, we use gmp.

#ifdef _MSC_VER
#include "mpir.h"
#else
#include "gmp.h"
#endif

class Erat_Mod;
class TreeFactorize;

struct FoundFactor
{
	uint64_t Factor;
	uint32_t exponent;
};

class NodeFactorize
{
	// NOTE only class TreeFactorize can create or use these, since ALL members (including construct/destruct) are private.
	friend class TreeFactorize;

	private:
		NodeFactorize();
		~NodeFactorize();

		void BuildTree(const char *Tree_Group, NodeFactorize *Parent, TreeFactorize *Root);
		void LoadPrimesIntoTree();
		void LoadPrimesIntoTree(Erat_Mod *pEratMod, Erat_Mod *pEratMod2);
		void ComputeTreeResidues(mpz_t Candi);
		void FillFactorFoundTable(uint64_t which, bool bDeep);

		mpz_t m_mpzData;
		mpz_t m_mpzResidue;
      mpz_t m_scrap;
		TreeFactorize *m_ftreeRoot;		// non-owned pointer
		NodeFactorize *m_fnodeParent;		// non-owned pointer
		NodeFactorize *m_fnodeBranches;	// Allocated and needs delete[]
		unsigned m_nNumBranches;		// If this is zero, then we are at a leaf node.
		FoundFactor *m_ffFactors;		// Allocated and needs delete[]

		unsigned m_nNumUsedFactors;		// WARNING, this value will be the total factoring in the TREE.  Thus, only a leaf will this number actually be used "against" the max value.
		unsigned m_nMaxLeafFactors;		// This the actual size of the m_ffFactors[] allocation (only valid on a leaf).
};

class TreeFactorize : public NodeFactorize
{
	friend class NodeFactorize;

	public:
		TreeFactorize(int MaxFoundFactors=1);
		~TreeFactorize();

		// Call build tree only one time (or whenever the size of the number
		// to factor significantly changes.
		// If a string is passed in for Tree_Group, use this format:
		//    leaves-node-node-node-node   
		//    So 26-4-3-2 would build a tree with 2 limbs. Each of those two
		//    limbs would be tree's with 3 limbs. each of those 6 limbs would
		//    be trees with 4 limbs.  each of those 24 limbs would be leaves
		//    which would hold 26 factors.  Total height is 4 (1(root), 2, 3, 4)
		// BuildTree has certain optimizations.  It will not rebuild unless 
		// something "changes".  Rebuilding is memory "clean". A single construction
		// of a TreeFactorize can be used over and over again, without mem loss.
		// Also multiple factor tree's can be working at any given time.
		bool BuildTree(mpz_t Candi, int64_t MaxPR, const char *Tree_Group=NULL);

		// Call this to load factor primes into the tree (using primegen). 
		// Returns the "ending" prime.  NOTE that StartingPrime is NOT set
		// The caller must initialize the primegen before starting the factoring,
		// but after that, it just "works"
		uint64_t LoadPrimesIntoTree();
		// This function used for "modular" sieve
		uint64_t LoadPrimesIntoTree(Erat_Mod *pEratMod, Erat_Mod *pEratMod2);

		// This will compute the full set of "residues" for the whole tree
		// If also sets m_bFound false, and clears the FoundFactor array.
		void ComputeTreeResidues(mpz_t Candi) { NodeFactorize::ComputeTreeResidues(Candi); }

		// This will compute the full set of "residues" for the whole tree
		// send 0 for factors, -1 or +1 for N-1 N+1 factors
		// if bDeep is false, then the first factor found will exit the whole works.
		void FillFactorFoundTable(int which, bool bDeep=false);

		unsigned NumFoundFactors() { return m_nNumFoundFactors; }
		const FoundFactor *GetFoundFactor(unsigned n) { if (n>=m_nNumFoundFactors) return 0; return &m_ffFoundFactors[n]; }

		const char *GetCurrentGroup(unsigned &nodes, unsigned &leaves, unsigned &height);
		unsigned GetNumLeaves() { return m_nLeaves; }
		bool HasGroupChanged() { return m_bNewTreeShape; }

		uint64_t MaxPrimeInTree() { return m_nCurPr; }

		void ResetToNoFactors() { m_nNumFoundFactors = 0; }

	private:
      mpz_t m_mpzCandi;
		mpz_t m_mpzScratch, m_mpzScratch2;
		bool m_bFound;
		FoundFactor *m_ffFoundFactors;	// Allocated and needs delete[]
		unsigned m_nMaxFoundFactors;
		unsigned m_nNumFoundFactors;
		unsigned m_nMaxLeafBits;
		char *m_szTreeGroup;			// Allocated and needs delete[]

		void AddFactor(const FoundFactor *f);

		uint64_t m_nCurPr;
		double m_dPrBits;

		unsigned m_nNodes;
		unsigned m_nLeaves;
		unsigned m_nHeight;

		unsigned m_nCandiBitsInitTree;

		bool	 m_bNewTreeShape;
		bool     m_bLastPrimeWasEratMod2;
};
