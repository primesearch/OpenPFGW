#include <math.h>
#include "pfoopch.h"
#include "treefactorize.h"
#include "../pflib/erat_mod.h"
#include "primeserver.h"

#ifndef _64BIT
extern "C" int32 Imod2(uint32 * const a,const int32 n1,const int32 n2,const int32 len, int32 *p1,int32 *p2);
#endif

///////////////////////////////////////////////////////////////
// Factor Tree logic.
//
// A Factor Tree is just a Factor Node that has additional
// some functionality, AND is the public interface
// into the whole "system".  All external interaction with
// the tree system should happen here.
//
// NOTE this class will track the "found" factors.  The class
// allocations enough found factors so that every "working"
// factor can be stored if need be.
///////////////////////////////////////////////////////////////
TreeFactorize::TreeFactorize(int MaxFoundFactors) : NodeFactorize()
{
	m_ffFoundFactors = NULL;
	m_bFound = false;
	m_nMaxFoundFactors = 0;
	m_nNumFoundFactors = 0;
	mpz_init(m_mpzCandi);
	// We want scratch and scratch 2 to have AT least 2 allocated limbs (we have some
	// optimizations for VC that depend upon this.
	mpz_init2(m_mpzScratch, 2*32-1);
	mpz_init2(m_mpzScratch2, 2*32-1);
	m_szTreeGroup = NULL;
	m_nCurPr = 0;
	m_dPrBits = 0.;

	m_nNodes = 0;
	m_nLeaves = 0;
	m_nHeight = 0;
	m_nMaxLeafBits = 0;
	m_nCandiBitsInitTree = 0;

	m_nMaxFoundFactors = MaxFoundFactors;
	m_ffFoundFactors = new FoundFactor[m_nMaxFoundFactors];

	// In the "real" PFGW, we probably also want some uint64 FoundFactor data arrays,
	// and probably also some Integer FoundFactor data arrays.  We would certainly
	// need a lot less of these arrays.

	m_bNewTreeShape = false;
	m_bLastPrimeWasEratMod2 = false;
}

TreeFactorize::~TreeFactorize()
{
	delete[] m_ffFoundFactors;
	delete[] m_szTreeGroup;
	mpz_clear(m_mpzScratch2);
	mpz_clear(m_mpzScratch);
	mpz_clear(m_mpzCandi);
}

// Call build tree only one time (or whenever the size of the number
// to factor significantly changes.
bool TreeFactorize::BuildTree(mpz_t Candi, int64 MaxPr, const char *UserDefinedTree)
{
	mpz_set(m_mpzCandi, Candi);
	m_dPrBits = log((double)MaxPr)/log(2.0);

	m_nNumFoundFactors = 0;

	m_bNewTreeShape = false;  // assume the tree maintains the same "shape"
	unsigned int CandiBits = (uint32) mpz_sizeinbase(Candi, 2);
	if ( (m_nCandiBitsInitTree!=0) && (abs((int) (CandiBits-m_nCandiBitsInitTree)) < 1000))
		return false;

	unsigned FactorsAtLeaf;
	if (!UserDefinedTree)
	{
		// Compute the group "ourselves", using Jen's algorithm, which seems close to optimal.
		unsigned leaves = (unsigned)(CandiBits/2/m_dPrBits);

		// The above is Jen's "original".  If we scale that value just a wee bit, we get a 
		// MUCH better (up to 25% at 1 million bits).   Also, Jen's code used an int for m_dPrBits
		// where we use a double (to get the actual value).  The int would make Jen's just a 
		// fraction larger.  However, using the double for PrBits and making the adjustment
		// below, it is pretty evened out.
		leaves = (unsigned)(((double)leaves*1.13));

		if (m_szTreeGroup && leaves == (unsigned)atoi(m_szTreeGroup))
			return false;  // still the same, so don't change.

		char Buf[2048];
		char *cp = Buf;
		unsigned treeheight = (int)(log((float) leaves)/log(2.0) - 3);
		if (((int)treeheight) <= 1)
			treeheight = 1;
		FactorsAtLeaf = leaves / (1 << (treeheight - 1));
		if (((int)FactorsAtLeaf) < 1)
			FactorsAtLeaf = 1;

		// Make them even (allows us to more easily use the m_mod2() function)  Round up.
		if (FactorsAtLeaf & 1)
			++FactorsAtLeaf;

		cp += sprintf(cp, "%d", FactorsAtLeaf);
		for (unsigned height = 1; height < treeheight; height++)
		{
			// At times these things help, but not that much.
//			if (height == 1 && treeheight > 6)
//				cp += sprintf(cp, ",3");
//			else if (height == 1 && treeheight > 3)
//				cp += sprintf(cp, ",3");
//			else
				cp += sprintf(cp, ",2");

			// NOTE this should never ever come close to this, but "just in case".
			if (cp-Buf > 2000)
				break;
		}

		// Are we "still" the same, if so, then to NOT create a new tree.
		if (m_szTreeGroup && !strcmp(m_szTreeGroup, Buf))
			return false;

		delete[] m_szTreeGroup;
		m_szTreeGroup = new char[strlen(Buf)+1];
		strcpy(m_szTreeGroup, Buf);

	}
	else
	{
		if (m_szTreeGroup && !strcmp(m_szTreeGroup, UserDefinedTree))
			return false;	// already setup correctly.

		// The user requested this factor tree, so lets use it.
		delete[] m_szTreeGroup;
		m_szTreeGroup = new char[strlen(UserDefinedTree)+1];
		strcpy(m_szTreeGroup, UserDefinedTree);
		FactorsAtLeaf = atoi(m_szTreeGroup);
	}

	// We are building a new tree, so flag what the params for this tree are.
	m_bNewTreeShape = true;
	m_nCandiBitsInitTree = CandiBits;

	// Since we are the root tree, then setup ourselves this way.
	m_fnodeParent = this;
	m_ftreeRoot = this;

	// Now, create the "other" trees.
	char *Buf = new char[strlen(m_szTreeGroup)+1];
	strcpy(Buf, m_szTreeGroup);
	char *cp = strrchr(Buf, ',');

	m_nNodes = 1;
	m_nHeight = 1;
	m_nLeaves = 0;
	if (!cp)
	{
		m_nNumBranches = 0;
		m_nNodes = 0;
		NodeFactorize::BuildTree(Buf, this, this);
		delete[] Buf;
		m_nNumUsedFactors = m_nLeaves;
		if (m_nNumUsedFactors > m_nMaxFoundFactors)
		{
			delete[] m_ffFoundFactors;
			m_nMaxFoundFactors = m_nNumUsedFactors + 50;
			m_ffFoundFactors = new FoundFactor[m_nMaxFoundFactors];
		}
		return true;
	}

	// There were some sub trees.  Set Cut off our number of branches, then allocate them
	// then create that many branches, using the "remainder" of the tree string.
	*cp++ = 0;

	m_nNumBranches = atoi(cp);

	// Compute the tree's height.
	char *cpTmp = strchr(m_szTreeGroup, ',');
	while (cpTmp)
	{
		++m_nHeight;
		cpTmp = strchr(&cpTmp[1], ',');
	}

	delete[] m_fnodeBranches;	// Avoid leaks when different tree "configurations" get built.
	m_fnodeBranches = new NodeFactorize[m_nNumBranches];
	for (unsigned  i = 0; i < m_nNumBranches; ++i)
	{
		m_fnodeBranches[i].BuildTree(Buf, this, this);
		if (m_fnodeBranches[i].m_nNumBranches)
			m_nNumUsedFactors += m_fnodeBranches[i].m_nNumUsedFactors;
		else
			m_nNumUsedFactors += m_fnodeBranches[i].m_nMaxLeafFactors;
	}

	delete[] Buf;
	if (m_nNumUsedFactors > m_nMaxFoundFactors)
	{
		delete[] m_ffFoundFactors;
		m_nMaxFoundFactors = m_nNumUsedFactors + 50;
		m_ffFoundFactors = new FoundFactor[m_nMaxFoundFactors];
	}

	m_nMaxLeafBits = (unsigned)(m_dPrBits * FactorsAtLeaf) + 1;

	return true;
}

// Call this to load factor primes into the tree (using primegen). 
// Returns the "ending" prime.  NOTE that StartingPrime is NOT the
// first prime used, but the prime just after StartingPrime is.  So
// to "really" start the whole process, start using 0, and then call
// LoadPrimesIntoTree() with the last return value + 1.
uint64 TreeFactorize::LoadPrimesIntoTree()
{
	NodeFactorize::LoadPrimesIntoTree();
	return m_nCurPr;
}

uint64 TreeFactorize::LoadPrimesIntoTree(Erat_Mod *pEratMod, Erat_Mod *pEratMod2)
{
	NodeFactorize::LoadPrimesIntoTree(pEratMod, pEratMod2);
	return m_nCurPr;
}

// send 0 for factors, -1 or +1 for N-1 N+1 factors
// if bDeep is false, then the first factor found will exit the whole works.
void TreeFactorize::FillFactorFoundTable(int which, bool bDeep)
{
	m_bFound = false;
	if (!m_nNumBranches)
	{
		NodeFactorize::FillFactorFoundTable(which, bDeep);
	}
	else
	{
		for (unsigned  i = 0; i < m_nNumBranches; ++i)
		{
			m_fnodeBranches[i].FillFactorFoundTable(which, bDeep);
			if (m_bFound && !bDeep)
				return;
		}
	}
}

// A leaf node found a factor, and is adding it here.  NOTE all
// "found" factors are stored by the root of the tree.
void TreeFactorize::AddFactor(const FoundFactor *f)
{
	if (m_nNumFoundFactors < m_nMaxFoundFactors)
	{
		m_ffFoundFactors[m_nNumFoundFactors].Factor = f->Factor;
		m_ffFoundFactors[m_nNumFoundFactors].exponent = f->exponent;
		++m_nNumFoundFactors;
	}
	m_bFound = true;
}

const char *TreeFactorize::GetCurrentGroup(unsigned &nodes, unsigned &leaves, unsigned &height)
{
	nodes=m_nNodes;
	leaves=m_nLeaves;
	height=m_nHeight;
	return m_szTreeGroup; 
}


//////////////////////////////////////////////////
// Factor NODE logic.
//
// A factor node is a tree in itself.  The root
// tree is a factor node (with special "external"
// logic.  The external program should only access
// throught the "root" node.   NOTE that a leaf node
// is still a node, but has no branches.  The leaf
// node is the only type of node where the m_ffFactors
// array is kept (each leaf tracks it's only smsll
// factor nodes).   
//////////////////////////////////////////////////

NodeFactorize::NodeFactorize()
{
	m_fnodeParent = 0;
	m_ftreeRoot = 0;
	// We want mpzData and mpzResidue to have AT least 2 allocated limbs (we have some
	// optimizations for VC that depend upon this.
	mpz_init2(m_mpzData, 2*32-1);
	mpz_init2(m_mpzResidue, 2*32-1);
   mpz_init(m_scrap);
	m_fnodeBranches = NULL;
	m_nNumBranches = 0;
	m_nNumUsedFactors = 0;
	m_nMaxLeafFactors = 0;
	m_ffFactors = NULL;
}

NodeFactorize::~NodeFactorize()
{
	delete[] m_fnodeBranches;
	delete[] m_ffFactors;
	mpz_clear(m_mpzData);
	mpz_clear(m_mpzResidue);
   mpz_clear(m_scrap);
}

void NodeFactorize::BuildTree(const char *Tree_Group, NodeFactorize *Parent, TreeFactorize *Root)
{
	m_fnodeParent = Parent;
	m_ftreeRoot = Root;
	++m_ftreeRoot->m_nNodes;

	char *Buf = new char[strlen(Tree_Group)+1];
	strcpy(Buf, Tree_Group);
	char *cp = strrchr(Buf, ',');
	if (!cp)
	{
		// Build a leaf
		m_nMaxLeafFactors = atoi(Buf);
		delete[] m_ffFactors;
		m_ffFactors = new FoundFactor[m_nMaxLeafFactors];
		delete[] Buf;

		m_ftreeRoot->m_nLeaves += m_nMaxLeafFactors;
		return;
	}
	// else a node
	*cp++ = 0;
	m_nNumBranches = atoi(cp);

	delete[] m_fnodeBranches;
	m_fnodeBranches = new NodeFactorize[m_nNumBranches];
	for (unsigned i = 0; i < m_nNumBranches; ++i)
	{
		m_fnodeBranches[i].BuildTree(Buf, this, m_ftreeRoot);
		if (m_fnodeBranches[i].m_nNumBranches)
			m_nNumUsedFactors += m_fnodeBranches[i].m_nNumUsedFactors;
		else
			m_nNumUsedFactors += m_fnodeBranches[i].m_nMaxLeafFactors;
	}
	delete[] Buf;
}

void NodeFactorize::LoadPrimesIntoTree(Erat_Mod *pEratMod, Erat_Mod *pEratMod2)
{
	if (m_nNumBranches)
	{
		m_fnodeBranches[0].LoadPrimesIntoTree(pEratMod, pEratMod2);
		mpz_set(m_mpzData, m_fnodeBranches[0].m_mpzData);
		for (unsigned i = 1; i < m_nNumBranches; ++i)
		{
			// Load each subtree.
			m_fnodeBranches[i].LoadPrimesIntoTree(pEratMod, pEratMod2);
			// Accumulate (muliply) the total factors of that subtree.
			mpz_mul(m_mpzData, m_mpzData, m_fnodeBranches[i].m_mpzData);
		}
		return;
	}
	// We are a leaf, so load up our factors, into the m_mpzData structure.
	uint32 i;
	m_nNumUsedFactors = m_nMaxLeafFactors;
	if (!pEratMod2)
	{
		for (i = 0; i < m_nMaxLeafFactors; ++i)
			m_ffFactors[i].Factor=pEratMod->next();
	}
	else
	{
		unsigned til = (m_nMaxLeafFactors>>1);
		for (i = 0; i < til; ++i)
		{
			m_ffFactors[i].Factor=pEratMod->next();
			m_ffFactors[i].Factor=pEratMod2->next();
		}
		if (m_nMaxLeafFactors & 1)
		{
			if (m_ftreeRoot->m_bLastPrimeWasEratMod2)
				m_ffFactors[m_nMaxLeafFactors-1].Factor=pEratMod->next();
			else
				m_ffFactors[m_nMaxLeafFactors-1].Factor=pEratMod2->next();
			m_ftreeRoot->m_bLastPrimeWasEratMod2 = !m_ftreeRoot->m_bLastPrimeWasEratMod2;

		}
	}
	if (m_ffFactors[m_nMaxLeafFactors-1].Factor < 0xFFFFFFFF)
	{
		mpz_set_ui(m_mpzData, (uint32)(m_ffFactors[0].Factor));
		for (i = 1; i < m_nMaxLeafFactors; ++i)
			mpz_mul_ui(m_mpzData, m_mpzData, (uint32)(m_ffFactors[i].Factor));
	}
#if defined (_MSC_VER)
	else if (m_ffFactors[0].Factor > 0xFFFFFFFF)
	{
		// NOTE this might be valid for any 32 bit GMP.  However, I am NOT doing this for
		// any GMP other than the VC version.  VC I KNOW works correctly.  However, I 
		// imagine that this "non-portable" method would work for ALL platforms PFGW
		// targets.
		*((uint64*)(&(m_mpzData->_mp_d[0]))) = m_ffFactors[0].Factor;
		m_mpzData->_mp_size = 2;
		m_ftreeRoot->m_mpzScratch->_mp_size = 2;
		for (i = 1; i < m_nMaxLeafFactors; ++i)
		{
			*((uint64*)(&(m_ftreeRoot->m_mpzScratch->_mp_d[0]))) = m_ffFactors[i].Factor;
			mpz_mul(m_mpzData, m_mpzData, m_ftreeRoot->m_mpzScratch);
		}
	}
#endif
	else  // handle the "transition", where some are over and some are under 32 bits.  For non-VC builds all above 32 bits are handled here
	{
		uint32 n32 = uint32((m_ffFactors[0].Factor)>>32);
		mpz_init_set_ui(m_mpzData,n32);
		mpz_mul_2exp(m_mpzData, m_mpzData, 32);
		mpz_add_ui(m_mpzData,m_mpzData,(uint32)((m_ffFactors[0].Factor)&0xFFFFFFFF));

		for (i = 1; i < m_nMaxLeafFactors; ++i)
		{
			n32 = uint32((m_ffFactors[i].Factor)>>32);
			mpz_init_set_ui(m_ftreeRoot->m_mpzScratch,n32);
			mpz_mul_2exp(m_ftreeRoot->m_mpzScratch, m_ftreeRoot->m_mpzScratch, 32);
			mpz_add_ui(m_ftreeRoot->m_mpzScratch,m_ftreeRoot->m_mpzScratch,(uint32)((m_ffFactors[i].Factor)&0xFFFFFFFF));
			mpz_mul(m_mpzData, m_mpzData, m_ftreeRoot->m_mpzScratch);
		}
	}
	// NOTE in DUAL Erat modular work, this max value is "not" quite right.
	// One or the other "dual" Erat mods, can get ahead of the other.  We simply
	// do NOT deal with that issue.  If we are sieving to 40 bits, and one gets
	// to 40 bits, while the other is only at 39.8bits, then the program will 
	// exit the factoring on that number (even though the trailing Erat mod
	// had not gotten there yet.  This is simply the way things work.  NOTE 
	// this "can" be an issue, if "re-factoring" later.   The "restart" would
	// not be right.   However, this issue will be lest "as it is", 
	m_ftreeRoot->m_nCurPr = m_ffFactors[m_nMaxLeafFactors-1].Factor;
}

void NodeFactorize::LoadPrimesIntoTree()
{
	if (m_nNumBranches)
	{
		m_fnodeBranches[0].LoadPrimesIntoTree();
		mpz_set(m_mpzData, m_fnodeBranches[0].m_mpzData);
		for (unsigned i = 1; i < m_nNumBranches; ++i)
		{
			// Load each subtree.
			m_fnodeBranches[i].LoadPrimesIntoTree();
			// Accumulate (muliply) the total factors of that subtree.
			mpz_mul(m_mpzData, m_mpzData, m_fnodeBranches[i].m_mpzData);
		}
		return;
	}
	// We are a leaf, so load up our factors, into the m_mpzData structure.
	uint32 i;
	m_nNumUsedFactors = m_nMaxLeafFactors;
	for (i = 0; i < m_nMaxLeafFactors; ++i)
		m_ffFactors[i].Factor = primeserver->NextPrime();
	if (m_ffFactors[m_nMaxLeafFactors-1].Factor < 0xFFFFFFFF)
	{
		mpz_set_ui(m_mpzData, (uint32)(m_ffFactors[0].Factor));
		for (i = 1; i < m_nMaxLeafFactors; ++i)
			mpz_mul_ui(m_mpzData, m_mpzData, (uint32)(m_ffFactors[i].Factor));
	}
#if defined (_MSC_VER)
	else if (m_ffFactors[0].Factor > 0xFFFFFFFF)
	{
		// NOTE this might be valid for any 32 bit GMP.  However, I am NOT doing this for
		// any GMP other than the VC version.  VC I KNOW works correctly.  However, I 
		// imagine that this "non-portable" method would work for ALL platforms PFGW
		// targets.
		*((uint64*)(&(m_mpzData->_mp_d[0]))) = m_ffFactors[0].Factor;
		m_mpzData->_mp_size = 2;
		m_ftreeRoot->m_mpzScratch->_mp_size = 2;
		for (i = 1; i < m_nMaxLeafFactors; ++i)
		{
			*((uint64*)(&(m_ftreeRoot->m_mpzScratch->_mp_d[0]))) = m_ffFactors[i].Factor;
			mpz_mul(m_mpzData, m_mpzData, m_ftreeRoot->m_mpzScratch);
		}
	}
#endif
	else  // handle the "transition", where some are over and some are under 32 bits.
	{
		uint32 n32 = uint32((m_ffFactors[0].Factor)>>32);
		mpz_init_set_ui(m_mpzData,n32);
		mpz_mul_2exp(m_mpzData, m_mpzData, 32);
		mpz_add_ui(m_mpzData,m_mpzData,(uint32)((m_ffFactors[0].Factor)&0xFFFFFFFF));
		for (i = 1; i < m_nMaxLeafFactors; ++i)
		{
			n32 = uint32((m_ffFactors[i].Factor)>>32);
			mpz_init_set_ui(m_ftreeRoot->m_mpzScratch,n32);
			mpz_mul_2exp(m_ftreeRoot->m_mpzScratch, m_ftreeRoot->m_mpzScratch, 32);
			mpz_add_ui(m_ftreeRoot->m_mpzScratch,m_ftreeRoot->m_mpzScratch,(uint32)((m_ffFactors[i].Factor)&0xFFFFFFFF));
			mpz_mul(m_mpzData, m_mpzData, m_ftreeRoot->m_mpzScratch);
		}
	}
	m_ftreeRoot->m_nCurPr = m_ffFactors[m_nMaxLeafFactors-1].Factor;
}

void NodeFactorize::FillFactorFoundTable(unsigned which, bool bDeep)
{
	unsigned i;
	if (m_nNumBranches)
	{
		for (i = 0; i < m_nNumBranches; ++i)
		{
			m_fnodeBranches[i].FillFactorFoundTable(which, bDeep);
			if (m_ftreeRoot->m_bFound && !bDeep)
				return;
		}
		return;
	}
	// To get here, we are a leaf.
	// We ONLY handle which of -1, 0, 1.

	if (which == 0 || which == 1)
	{
		if (m_ffFactors[m_nNumUsedFactors-1].Factor < 0xFFFFFFFF)
		{
			if (m_ffFactors[m_nNumUsedFactors-1].Factor < 0x7FFFFFFF)
			{
				// m_nNumUsedFactors must be even for this to work right (without having to handle 
				// the extraneous trailing factor.  Thus, during construction, we DO make all
				// trees have an even (rounded up) number of factors in the leaves.
				unsigned til = m_nNumUsedFactors>>1;

				int32  r1, r2;
				uint32 p1, p2;

				// Note if the FP stack is not PERFECTLY clear before calling this function, then
				// there it will have problems and not work right (this was found inside of APSieve).
				// I think this bug may ALSO be in pfgw in some instances during factoring.
				for (i = 0; i < til; ++i)
				{
					p1 = (uint32)(uint32)(m_ffFactors[i<<1].Factor);
					p2 = (uint32)(uint32)(m_ffFactors[(i<<1)+1].Factor);

#if defined(_64BIT)
               // This only works if p1 and p2 < 32 bits due to Windows-isms.  See integer.inl for more info.
               r1 = (int32) mpz_mod_ui(*(mpz_t*)(&m_scrap),m_mpzResidue,p1);
	            r2 = (int32) mpz_mod_ui(*(mpz_t*)(&m_scrap),m_mpzResidue,p2);
#else
					Imod2((uint32*)(m_mpzResidue->_mp_d),(int32)p1,(int32)p2,m_mpzResidue->_mp_size,&r1,&r2);
#endif

					// We don't care if R1 or R2 is less than 0.  If so, then the value will NEVER
					// be 0 or 1 (the 2 values we care about here.)

					if (((uint32)r1) == which)
					{
						m_ftreeRoot->AddFactor(&m_ffFactors[i<<1]);
						if (!bDeep)
							return;
					}
					if (((uint32)r2) == which)
					{
						m_ftreeRoot->AddFactor(&m_ffFactors[(i<<1)+1]);
						if (!bDeep)
							return;
					}
				}
			}
			else
			{
				for (i = 0; i < m_nNumUsedFactors; ++i)
				{
					unsigned res = mpz_tdiv_ui(m_mpzResidue, (uint32)(m_ffFactors[i].Factor));
					if (res == which)
					{
						m_ftreeRoot->AddFactor(&m_ffFactors[i]);
						if (!bDeep)
							return;
					}
				}
			}
		}
#if defined (_MSC_VER)
		else if (m_ffFactors[0].Factor > 0xFFFFFFFF)
		{
			for (i = 0; i < m_nNumUsedFactors; ++i)
			{
				*((uint64*)(&(m_ftreeRoot->m_mpzScratch->_mp_d[0]))) = m_ffFactors[i].Factor;
				m_ftreeRoot->m_mpzScratch->_mp_size = 2;

				mpz_tdiv_r(m_ftreeRoot->m_mpzScratch, m_mpzResidue, m_ftreeRoot->m_mpzScratch);
				if (mpz_cmp_ui(m_ftreeRoot->m_mpzScratch, which) == 0)
				{
					m_ftreeRoot->AddFactor(&m_ffFactors[i]);
					if (!bDeep)
						return;
				}
			}

		}
#endif
		else
		{
			for (i = 0; i < m_nNumUsedFactors; ++i)
			{
				// For VC, we only use this code for the "transition" from 32 to 64 bit.  Once we are
				// fully into 64 bit land (i.e. 2 limbs for a GMP), we use the code in the #if above
				uint32 n32 = uint32((m_ffFactors[i].Factor)>>32);
				mpz_init_set_ui(m_ftreeRoot->m_mpzScratch,n32);
				mpz_mul_2exp(m_ftreeRoot->m_mpzScratch, m_ftreeRoot->m_mpzScratch, 32);
				mpz_add_ui(m_ftreeRoot->m_mpzScratch,m_ftreeRoot->m_mpzScratch,(uint32)((m_ffFactors[i].Factor)&0xFFFFFFFF));

				mpz_tdiv_r(m_ftreeRoot->m_mpzScratch, m_mpzResidue, m_ftreeRoot->m_mpzScratch);
				if (mpz_cmp_ui(m_ftreeRoot->m_mpzScratch, which) == 0)
				{
					m_ftreeRoot->AddFactor(&m_ffFactors[i]);
					if (!bDeep)
						return;
				}
			}
		}
		return;
	}

	// Which is assumed to be -1
	if (m_ffFactors[m_nNumUsedFactors-1].Factor < 0xFFFFFFFF)
	{
		if (m_ffFactors[m_nNumUsedFactors-1].Factor < 0x7FFFFFFF)
		{
			// m_nNumUsedFactors must be even for this to work right (without having to handle 
			// the extraneous trailing factor.  Thus, during construction, we DO make all
			// trees have an even (rounded up) number of factors in the leaves.
			unsigned til = m_nNumUsedFactors>>1;

			int32  r1, r2;
			uint32 p1, p2;

			// Note if the FP stack is not PERFECTLY clear before calling this function, then
			// there it will have problems and not work right (this was found inside of APSieve).
			// I think this bug may ALSO be in pfgw in some instances during factoring.
			for (i = 0; i < til; ++i)
			{
				p1 = (uint32)(m_ffFactors[i<<1].Factor);
				p2 = (uint32)(m_ffFactors[(i<<1)+1].Factor);

#if defined(_64BIT)
               // This only works if p1 and p2 < 32 bits due to Windows-isms.  See integer.inl for more info.
               r1 = (int32) mpz_mod_ui(*(mpz_t*)(&m_scrap),m_mpzResidue,p1);
	            r2 = (int32) mpz_mod_ui(*(mpz_t*)(&m_scrap),m_mpzResidue,p2);
#else
				Imod2((uint32*)(m_mpzResidue->_mp_d),(int32)p1,(int32)p2,m_mpzResidue->_mp_size,&r1,&r2);
#endif

				// This is the only way I can find to handle 2.  the value returned for 2 is 1.  However,
				// the value for all others is -1.   We do lose just a "little" of the benefit of the 
				// Imod2() code (we lose about 10% of our gain), but doing this any other way is not
				// as safe.  2 is the only value I currently KNOW that does this, but that does not
				// mean that there are not others.  The below code works for the case of 2, and for
				// any other that happens to happen.
				if(r1>0) 
					r1-=p1;
				if(r2>0) 
					r2-=p2;
				if (r1 == -1)
				{
					m_ftreeRoot->AddFactor(&m_ffFactors[i<<1]);
					if (!bDeep)
						return;
				}
				if (r2 == -1)
				{
					m_ftreeRoot->AddFactor(&m_ffFactors[(i<<1)+1]);
					if (!bDeep)
						return;
				}
			}
		}
		else
		{
			for (i = 0; i < m_nNumUsedFactors; ++i)
			{
				unsigned res = mpz_tdiv_ui(m_mpzResidue, (uint32)(m_ffFactors[i].Factor));
				if (res == m_ffFactors[i].Factor-1)
				{
					m_ftreeRoot->AddFactor(&m_ffFactors[i]);
					if (!bDeep)
						return;
				}
			}
		}
	}
#if defined (_MSC_VER)
	else if (m_ffFactors[0].Factor > 0xFFFFFFFF)
	{
		for (i = 0; i < m_nNumUsedFactors; ++i)
		{
			*((uint64*)(&(m_ftreeRoot->m_mpzScratch->_mp_d[0]))) = m_ffFactors[i].Factor;
			m_ftreeRoot->m_mpzScratch->_mp_size = 2;

			mpz_tdiv_r(m_ftreeRoot->m_mpzScratch2, m_mpzResidue, m_ftreeRoot->m_mpzScratch);
			mpz_sub_ui(m_ftreeRoot->m_mpzScratch, m_ftreeRoot->m_mpzScratch, 1);
			if (mpz_cmp(m_ftreeRoot->m_mpzScratch2, m_ftreeRoot->m_mpzScratch) == 0)
			{
				m_ftreeRoot->AddFactor(&m_ffFactors[i]);
				if (!bDeep)
					return;
			}
		}
	}
#endif
	else
	{
		for (i = 0; i < m_nNumUsedFactors; ++i)
		{
			uint32 n32 = uint32((m_ffFactors[i].Factor)>>32);
			mpz_init_set_ui(m_ftreeRoot->m_mpzScratch,n32);
			mpz_mul_2exp(m_ftreeRoot->m_mpzScratch, m_ftreeRoot->m_mpzScratch, 32);
			mpz_add_ui(m_ftreeRoot->m_mpzScratch,m_ftreeRoot->m_mpzScratch,(uint32)((m_ffFactors[i].Factor)&0xFFFFFFFF));

			mpz_tdiv_r(m_ftreeRoot->m_mpzScratch2, m_mpzResidue, m_ftreeRoot->m_mpzScratch);
			mpz_sub_ui(m_ftreeRoot->m_mpzScratch, m_ftreeRoot->m_mpzScratch, 1);
			if (mpz_cmp(m_ftreeRoot->m_mpzScratch2, m_ftreeRoot->m_mpzScratch) == 0)
			{
				m_ftreeRoot->AddFactor(&m_ffFactors[i]);
				if (!bDeep)
					return;
			}
		}
	}
}

// This will compute the full set of "residues" for the whole tree
// If also sets m_bFound false, and clears the FoundFactor array.
void NodeFactorize::ComputeTreeResidues(mpz_t Candi)
{
   unsigned int i;

   mpz_fdiv_r(m_mpzResidue, Candi, m_mpzData);
	for (i = 0; i < m_nNumBranches; ++i)
		m_fnodeBranches[i].ComputeTreeResidues(m_mpzResidue);
}
