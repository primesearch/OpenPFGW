#include "pfoopch.h"
#include "algebra.h"

#undef GWDEBUG
#undef INTDEBUG
#define GWDEBUG(X) {Integer XX;XX=X;printf(#X "=");mpz_out_str(stdout,16,XX.gmp());printf("\n");}
#define INTDEBUG(X) {printf(#X "=");mpz_out_str(stdout,16,X.gmp());printf("\n");}

extern bool volatile g_bExitNow;

FiniteField::~FiniteField()
{
}

Residue::~Residue()
{
}

Multiplier::~Multiplier()
{
}

void Residue::squaremultiply(Multiplier *m, int maxSteps, int stepsLeft)
{
	square(maxSteps, stepsLeft);
	multiply(m);
}

//==============================================================
// The finite field Z/nZ.
//==============================================================

FieldZ::FieldZ(Integer *N)
{
   gwinit2(&gwdata, sizeof(gwhandle), GWNUM_VERSION);
   if (gwdata.GWERROR == GWERROR_VERSION_MISMATCH)
   {
		PFOutput::EnableOneLineForceScreenOutput();
		PFPrintfStderr ("GWNUM version mismatch.  PFGW is not linked with version %s of GWNUM.\n", GWNUM_VERSION);
      g_bExitNow = true;
   }

   if (gwdata.GWERROR == GWERROR_STRUCT_SIZE_MISMATCH)
   {
		PFOutput::EnableOneLineForceScreenOutput();
		PFPrintfStderr ("GWNUM struct size mismatch.  PFGW must be compiled with same switches as GWNUM.\n");
      g_bExitNow = true;
   }

   gwsetmaxmulbyconst(&gwdata, GWMULBYCONST_MAX);	// maximum multiplier
	CreateModulus(N);
}

FieldZ::~FieldZ()
{
	DestroyModulus();
}

// fieldZ is basically a factory that lets you create derived
// classes. That's the whole point of its existence

Multiplier *FieldZ::createCompatibleMultiplier(Integer &N)
{
	Multiplier *pRetval=NULL;

	// check for small multipliers (<8 bits)	
	int l=N&0x000000FF;
	if(N==Integer(l))
	{
		pRetval=new SmallIntegerMultiplier(l);
	}
	else
	{
		pRetval=new IntegerMultiplier(N);
	}
	
	return pRetval;
}

Residue *FieldZ::createCompatibleResidue(Integer &N)
{
    return new IntegerResidue(N);
}

// and here are the things it can create

IntegerResidue::IntegerResidue(Integer &N)
{
	R = N;
}

IntegerResidue::IntegerResidue(GWInteger &RR) : R(RR)
{
}

IntegerResidue::~IntegerResidue()
{
}

Residue *IntegerResidue::clone()
{
    return new IntegerResidue(R);
}

Multiplier *IntegerResidue::cloneMul()
{
    return new IntegerMultiplier(R);
}

void IntegerResidue::multiply(Multiplier *x)
{
    FieldZMultiplier *f=(FieldZMultiplier*)x;   // cast, ok since we know compatible
    f->mulInteger(R);
}

OutputResidue *IntegerResidue::collapse()
{
	IntegerOutputResidue *p=new IntegerOutputResidue;
	p->content() = R;
	return p;
}

void IntegerResidue::square(int maxSteps, int stepsLeft)
{
	gwsetnormroutine(&gwdata,0,g_bErrorCheckAllTests,0);
   // Square carefully the first 30 and last 30 steps
   if (maxSteps - stepsLeft < 30 || stepsLeft < 30)
      gwsquare_carefully(R);
   else
   	gwsquare(R);
}

void IntegerResidue::squaremultiply(Multiplier *l, int maxSteps, int stepsLeft)
{
    FieldZMultiplier *f=(FieldZMultiplier*)l;   // cast, ok since we know compatible
    f->squaremulInteger(R, maxSteps, stepsLeft);
}

IntegerOutputResidue::IntegerOutputResidue()
	: R(0)
{
}

Integer &IntegerOutputResidue::content()
{
	return R;
}

IntegerMultiplier::IntegerMultiplier(GWInteger &MM)
	: M(MM)
{
	gwfft(M,M);
}

IntegerMultiplier::IntegerMultiplier(Integer &N)
	: M()
{
	M = N;
	gwfft(M,M);
}

IntegerMultiplier::~IntegerMultiplier()
{
}

void IntegerMultiplier::mulInteger(GWInteger &X)
{
	gwsetnormroutine(&gwdata,0,g_bErrorCheckAllTests,0);
	gwfftmul(M,X);
}

void IntegerMultiplier::squaremulInteger(GWInteger &X, int maxSteps, int stepsLeft)
{
	gwsetnormroutine(&gwdata,0,g_bErrorCheckAllTests,0);
   // Square carefully the first 30 and last 30 steps
   if (maxSteps - stepsLeft < 30 || stepsLeft < 30)
      gwsquare_carefully(X);
   else
   	gwsquare(X);
	gwsetnormroutine(&gwdata,0,g_bErrorCheckAllTests,0);
	gwfftmul(M,X);
}

SmallIntegerMultiplier::SmallIntegerMultiplier(int mmm) : mm(mmm)
{
}

SmallIntegerMultiplier::~SmallIntegerMultiplier()
{
}

void SmallIntegerMultiplier::mulInteger(GWInteger &X)
{
	// note this routine is *very* slow, requiring a full FFT
	// every time it is called. Short answer, don't call it.
	// it will always be used through squaremul
	GWInteger gwM;
	gwM=mm;
	
	gwsetnormroutine(&gwdata,0,g_bErrorCheckAllTests,0);
	gwfftmul(gwM,X);
}

void SmallIntegerMultiplier::squaremulInteger(GWInteger &X, int maxSteps, int stepsLeft)
{
	gwsetmulbyconst(&gwdata,mm);
	gwsetnormroutine(&gwdata,0,g_bErrorCheckAllTests,1);
   // Square carefully the first 30 and last 30 steps
   if (maxSteps - stepsLeft < 30 || stepsLeft < 30)
      gwsquare_carefully(X);
   else
   	gwsquare(X);
}

//==============================================================
// The Lucas field Z/nZ(sqrt(D)). Of course, we're just *hoping*
// it's a field (it's only a ring, really, unless N is prime)
// but we might as well pretend it's a ring for the moment
//==============================================================

FieldLucas::FieldLucas(Integer *N)
	: ps1(NULL), ps2(NULL), ps3(NULL), ps4(NULL)
{
   gwinit2(&gwdata, sizeof(gwhandle), GWNUM_VERSION);
   if (gwdata.GWERROR == GWERROR_VERSION_MISMATCH)
   {
		PFOutput::EnableOneLineForceScreenOutput();
		PFPrintfStderr ("GWNUM version mismatch.  PFGW is not linked with version %s of GWNUM.\n", GWNUM_VERSION);
      g_bExitNow = true;
   }

   if (gwdata.GWERROR == GWERROR_STRUCT_SIZE_MISMATCH)
   {
		PFOutput::EnableOneLineForceScreenOutput();
		PFPrintfStderr ("GWNUM struct size mismatch.  PFGW must be compiled with same switches as GWNUM.\n");
      g_bExitNow = true;
   }

	gwsetmaxmulbyconst(&gwdata, GWMULBYCONST_MAX);	// maximum multiplier
	CreateModulus(N);
	
	ps1=new GWInteger;
	ps2=new GWInteger;
	ps3=new GWInteger;
	ps4=new GWInteger;
}

FieldLucas::~FieldLucas()
{
	delete ps1;
	delete ps2;
	delete ps3;
	delete ps4;
	DestroyModulus();
}

// we must check P has no common factor with Q. Note that since
// P is odd, this is the same as checking P has no common factor
// with D

Multiplier *FieldLucas::createCompatibleMultiplier(Integer &VV,Integer &UU)
{
	// decide whether the thing can be made wide or narrow
	Multiplier *pRetval=NULL;
	int iV=VV&0xFFFFFFFF;
	int iU=UU&0xFFFFFFFF;
	
	if((VV==iV)&&(UU==iU))
	{
		if( (double)iV>-GWSMALLMUL_MAX && (double)iV<GWSMALLMUL_MAX &&
		    (double)iU>-GWSMALLMUL_MAX && (double)iU<GWSMALLMUL_MAX )
		{
			pRetval=new NarrowLucasMultiplier((double)iV,(double)iU);
		}
	}
	
	if(pRetval==NULL)
	{
		pRetval=new WideLucasMultiplier(VV,UU);
	}
	
	return pRetval;
}

Residue *FieldLucas::createCompatibleResidue(Integer &VV, Integer &UU)
{
    return new IntegerLucasResidue(this,VV,UU);
}

// FieldLucasSmall and large variants
FieldLucasSmall::FieldLucasSmall(int DD,Integer *N)
	: FieldLucas(N), D(DD)
{
}

FieldLucasSmall::~FieldLucasSmall()
{
}

void FieldLucasSmall::mulcross(FieldLucasMultiplier *mm,GWInteger &u)
{
	// mulcross computes 2(D-1)ua
	mm->mulad(u,2*(D-1));
}

void FieldLucasSmall::mulcross2(FieldLucasMultiplier *mm,GWInteger &u)
{
	// mulcross computes (D-1)ua
	mm->mulad(u,(D-1));
}

void FieldLucasSmall::squarecross(GWInteger &ufft)
{
	// 2Du^2
	gwsetnormroutine(&gwdata,0,g_bErrorCheckAllTests,1);
	gwsetmulbyconst(&gwdata,D*2);
	gwfftfftmul(ufft,ufft,ufft);
}

void FieldLucasSmall::squaremulcross(GWInteger &u, int maxSteps, int stepsLeft)
{
	// 2(D-1)u^2
	gwsetnormroutine(&gwdata,0,g_bErrorCheckAllTests,1);		// b(u) = n+2
	gwsetmulbyconst(&gwdata,(D-1)*2);	
   // Square carefully the first 30 and last 30 steps
   if (maxSteps - stepsLeft < 30 || stepsLeft < 30)
      gwsquare_carefully(u);
   else
   	gwsquare(u);
}	

// A 'medium' has a possibly 32-bit multiplier rather than an 8 bit one, so
// be sure to allocate enough extra bits
FieldLucasMedium::FieldLucasMedium(double DD,Integer *N)
	: FieldLucas(N), D(DD)
{
}

FieldLucasMedium::~FieldLucasMedium()
{
}

void FieldLucasMedium::mulcross(FieldLucasMultiplier *mm,GWInteger &u)
{
	// mulcross computes 2(D-1)ua
	mm->mulad(u,2.0*(D-1.0));
}

void FieldLucasMedium::mulcross2(FieldLucasMultiplier *mm,GWInteger &u)
{
	// mulcross computes (D-1)ua
	mm->mulad(u,(D-1.0));
}

void FieldLucasMedium::squarecross(GWInteger &ufft)
{
	// 2Du^2
	gwsetnormroutine(&gwdata,0,g_bErrorCheckAllTests,0);
	gwfftfftmul(ufft,ufft,ufft);
	gwsmallmul(D*2.0,ufft);
}

void FieldLucasMedium::squaremulcross(GWInteger &u, int maxSteps, int stepsLeft)
{
	// 2(D-1)u^2
	gwsetnormroutine(&gwdata,0,g_bErrorCheckAllTests,0);		// b(u) = n+2
   // Square carefully the first 30 and last 30 steps
   if (maxSteps - stepsLeft < 30 || stepsLeft < 30)
      gwsquare_carefully(u);
   else
   	gwsquare(u);					// b(u) = 2n+d+5
	gwsmallmul(2.0*(D-1.0),u);
}

// FieldLucasSmall and large variants
FieldLucasLarge::FieldLucasLarge(Integer &DD,Integer *N)
	: FieldLucas(N), dminus1(), twod()
{
	Integer D2(DD-1);

	dminus1 = D2;
	gwfft(dminus1,dminus1);
	++D2;
	D2<<=1;
	twod = D2;
	gwfft(twod,twod);
}

FieldLucasLarge::~FieldLucasLarge()
{
}

void FieldLucasLarge::mulcross(FieldLucasMultiplier *mm,GWInteger &u)
{
	// mulcross computes 2(D-1)ua
	mm->mulad(u,dminus1,2.0);

}

void FieldLucasLarge::mulcross2(FieldLucasMultiplier *mm,GWInteger &u)
{
	// mulcross computes (D-1)ua
	mm->mulad(u,dminus1,1.0);
}

void FieldLucasLarge::squarecross(GWInteger &ufft)
{
	// 2Du^2
	gwsetnormroutine(&gwdata,0,g_bErrorCheckAllTests,0);
	gwfftfftmul(ufft,ufft,ufft);
	gwsetnormroutine(&gwdata,0,g_bErrorCheckAllTests,0);
	gwfftmul(twod,ufft);
}

void FieldLucasLarge::squaremulcross(GWInteger &u, int maxSteps, int stepsLeft)
{
	// 2(D-1)u^2
	gwsetnormroutine(&gwdata,0,g_bErrorCheckAllTests,0);	// b(u) n+2
   // Square carefully the first 30 and last 30 steps
   if (maxSteps - stepsLeft < 30 || stepsLeft < 30)
      gwsquare_carefully(u);
   else
   	gwsquare(u);							// b(u) 2n+4
										// b(u) n+2
	gwsetnormroutine(&gwdata,0,g_bErrorCheckAllTests,1);
	gwsetmulbyconst(&gwdata,2);
	gwfftmul(dminus1,u);						// b(u) n+d+3
}	


IntegerLucasResidue::IntegerLucasResidue(FieldLucas *ff,Integer &VV,Integer &UU)
    : f(ff), v(), u()
{
	v = VV;
	u = UU;
}

IntegerLucasResidue::IntegerLucasResidue(FieldLucas *ff,GWInteger &VV,GWInteger &UU)
    : f(ff), v(VV), u(UU)
{
}

IntegerLucasResidue::~IntegerLucasResidue()
{
}

Residue *IntegerLucasResidue::clone()
{
    return new IntegerLucasResidue(f,v,u);
}

Multiplier *IntegerLucasResidue::cloneMul()
{
    return new WideLucasMultiplier(v,u);
}

#define s1 		*f->ps1
#define s2 		*f->ps2
#define s3 		*f->ps3
#define s4		*f->ps4

void IntegerLucasResidue::multiply(Multiplier *x)
{
// time to write the guts. Yeuck.
// Then let s1=(v+u)(b+a), s2=(v-u)(b-a)
// and so y=(s1+s2)+2(D-1)ua
// and x=(s1-s2).
	gwaddsub4(v,u,s1,s2);									// v=v+u, u=v-u
	((FieldLucasMultiplier*)x)->mulResidues(s1,s2);			// multiplies but doesn't reduce?
	f->mulcross(((FieldLucasMultiplier*)x),u);
	gwaddsub(s1,s2);
	gwadd3(s1,u,v);
	gwcopy(s2,u);
}

void IntegerLucasResidue::square(int maxSteps, int stepsLeft)
{
// uninspired method suggests
// v' = 2v^2 + 2Du^2
// u' = 4uv
   if (maxSteps - stepsLeft < 30 || stepsLeft < 30)
   { 
	   gwfft(u,s1);                  // s1 is the FFT of u 
	   gwcopy(v,s2); 
	   gwsetnormroutine(&gwdata,0,g_bErrorCheckAllTests,1); 
	   gwsetmulbyconst(&gwdata,2); 
	   gwsquare_carefully(v);        // v' is 2v^2 
	   gwsetmulbyconst(&gwdata,4); 
	   gwmul_carefully(s2,u);        // u' is 4uv 
   }
   else
   { 
	   gwfft(u,s1);                  // s1 is the FFT of u 
	   gwfft(v,u);	                  // u is the FFT of v 
	   gwsetnormroutine(&gwdata,0,g_bErrorCheckAllTests,1); 
	   gwsetmulbyconst(&gwdata,2); 
	   gwfftfftmul(u,u,v);		      // v' is 2v^2 
	   gwsetmulbyconst(&gwdata,4); 
	   gwfftfftmul(s1,u,u);		      // u' is 4uv 
   }

	f->squarecross(s1);
	gwadd(s1, v);
}

#define DEBUGBITS(X)	{Integer XX;XX=X;printf(#X ":%ld\n",lg(XX+1));}

void IntegerLucasResidue::squaremultiply(Multiplier *x, int maxSteps, int stepsLeft)
{
	gwcopy(u,s1);										// b(u) n+2 b(v) n+2 b(s1) n+2
	f->squaremulcross(s1, maxSteps, stepsLeft);   // b(s1) 2n+d+5 or n+d+3
	gwaddsub(v,u);										// b(u) n+3 b(v) n+3

	gwsetnormroutine(&gwdata,0,g_bErrorCheckAllTests,1);
	gwsetmulbyconst(&gwdata,2);
   if (maxSteps - stepsLeft < 30 || stepsLeft < 30)
   {
   	gwsquare_carefully(v);						// b(u) 2n+7 b(v) 2n+7
	   gwsquare_carefully(u);
   }
   else
   {
   	gwsquare(v);									// b(u) 2n+7 b(v) 2n+7
	   gwsquare(u);
   }
	gwadd(s1,v);										// b(u) 2n+8 2n+d+6 n+d+4
	gwadd(s1,u);										// b(v) 2n+8 2n+d+6 n+d+4
				// v'+u'
				// v'-u'
	
	gwsub3(v,u,s1);										// n+t+1
	f->mulcross2(((FieldLucasMultiplier*)x),s1);		// n+t+7

	((FieldLucasMultiplier*)x)->mulResidues(v,u);		// 2n+2t+1
	gwaddsub(v,u);										// 2n+2t+2
	gwadd(s1,v);										// 2n+2t+3

// Then let s1=(v+u)(b+a), s2=(v-u)(b-a)
// and so y=(s1+s2)+2(D-1)ua
// and x=(s1-s2).
}

OutputResidue *IntegerLucasResidue::collapse()
{
	IntegerLucasOutputResidue *pRetval=new IntegerLucasOutputResidue;
	pRetval->content() = u;
//	GWDEBUG(u);
	return pRetval;
}

IntegerLucasOutputResidue::IntegerLucasOutputResidue()
	: U(0)
{
}

Integer &IntegerLucasOutputResidue::content()
{
	return U;
}

WideLucasMultiplier::WideLucasMultiplier(Integer &BB,Integer &AA)
    : bpa(), bma(), a()
{
	bpa = BB;
	bma = AA;
	
	gwcopy(bma,a);
	gwaddsub(bpa,bma);
	
	gwfft(bpa,bpa);				// n+t+1 bits
	gwfft(bma,bma);				// n+t+1 bits
	gwfft(a,a);					// n+t bits
}

WideLucasMultiplier::WideLucasMultiplier(GWInteger &BB,GWInteger &AA)
    : bpa(BB), bma(AA), a(AA)
{
	gwaddsub(bpa,bma);
	gwfft(bpa,bpa);
	gwfft(bma,bma);
	gwfft(a,a);
}

WideLucasMultiplier::~WideLucasMultiplier()
{
}

void WideLucasMultiplier::mulResidues(GWInteger &v,GWInteger &u)
{
	gwsetnormroutine(&gwdata,0,g_bErrorCheckAllTests,0);
	gwfftmul(bpa,v);
	gwfftmul(bma,u);
}	

void WideLucasMultiplier::mulad(GWInteger &u,int D)
{
	gwsetnormroutine(&gwdata,0,g_bErrorCheckAllTests,1);
	gwsetmulbyconst(&gwdata,D);
	gwfftmul(a,u);
}

void WideLucasMultiplier::mulad(GWInteger &u,double D)
{
	gwsetnormroutine(&gwdata,0,g_bErrorCheckAllTests,0);
	gwfftmul(a,u);
	gwsmallmul(D,u);
}

void WideLucasMultiplier::mulad(GWInteger &u,GWInteger &dfft,double dd)
{
	if(dd!=1.0)
	{
		gwsetnormroutine(&gwdata,0,g_bErrorCheckAllTests,1);
		gwsetmulbyconst(&gwdata,(int)(dd));
	}
	else
	{
		gwsetnormroutine(&gwdata,0,g_bErrorCheckAllTests,0);
	}
	gwfftmul(a,u);
	gwsetnormroutine(&gwdata,0,g_bErrorCheckAllTests,0);
	gwfftmul(dfft,u);
}

NarrowLucasMultiplier::NarrowLucasMultiplier(double BB,double AA)
	: bpa(BB+AA), bma(BB-AA), a(AA)
{
}

NarrowLucasMultiplier::~NarrowLucasMultiplier()
{
}

void NarrowLucasMultiplier::mulResidues(GWInteger &v,GWInteger &u)
{
	gwsmallmul(bpa,v);
	if(bma==0.0)
	{
		u=0;
	}
	else if(bma==1.0)
	{
	}
	else
	{
		gwsmallmul(bma,u);
	}
}

void NarrowLucasMultiplier::mulad(GWInteger &u,int D)
{
	double dQuick=a*D;
	if(dQuick>-GWSMALLMUL_MAX && dQuick<GWSMALLMUL_MAX)
	{
		gwsmallmul(dQuick,u);
	}
	else
	{
		gwsmallmul(D,u);
		if(a!=1.0)
		{
			gwsmallmul(a,u);
		}
	}
}

void NarrowLucasMultiplier::mulad(GWInteger &u,double D)
{
	double dQuick=a*D;
	if(dQuick>-GWSMALLMUL_MAX && dQuick<GWSMALLMUL_MAX)
	{
		gwsmallmul(dQuick,u);
	}
	else
	{
		gwsmallmul(D,u);
		if(a!=1.0)
		{
			gwsmallmul(a,u);
		}
	}
}

void NarrowLucasMultiplier::mulad(GWInteger &u,GWInteger &dfft,double dd)
{
	double aa=a*dd;
	
	if(aa>=-GWMULBYCONST_MAX && aa<=GWMULBYCONST_MAX)
	{
		gwsetnormroutine(&gwdata,0,g_bErrorCheckAllTests,1);
		gwsetmulbyconst(&gwdata,(int)(aa));
	}
	else
	{
		if(aa!=1.0)
		{
			gwsmallmul(aa,u);
		}
		gwsetnormroutine(&gwdata,0,g_bErrorCheckAllTests,0);
	}
					
	gwfftmul(dfft,u);
}

