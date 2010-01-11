#include "pfiopch.h"
#include <string.h>
#include <math.h>
#include "pfabc2file.h"

PFABC2File::PFABC2File(const char* FileName)
	: PFABCFile(FileName), m_nFirstPrime(-1)
{
	for (int i = 0; i < 26; i++)
		m_pSet[i] = 0;
	m_SigString = "ABC2 File";
}

PFABC2File::~PFABC2File()
{
	for (int i = 0; i < 26; i++)
	{
		if (m_pSet[i])
		{
			int nMax = (int)(max[i] + 0.5)+1;
			for (int k = 0; k < nMax; k++)
				delete[] m_pSet[i][k];
			free(m_pSet[i]);
		}
	}
}

void PFABC2File::LoadFirstLine()
{
	PFABCFile::LoadFirstLine();

	int i,j,k,temp;	
	char Letter[10];
	char *HoldSet[1000];
	char *tempPtr;
	char *tmpLineBuf = new char[ABCLINELEN];

	for (j = 0; j < 1000; ++j)
		HoldSet[j] = 0;

	for (i=0;i<=m_nLastLetter;i++)
	{
		if (ReadLine(Line, ABCLINELEN)) 
		{
			if (*Line==0)
			{
				delete[] tmpLineBuf;
				throw "Not enough range information in ABC2 file";
			}
		}
		temp = 0;
		if (strstr(Line, " downto "))
		{
			// NOTE min and max are "reversed" due to min being larger than max.  Also, step MUST be negative.
			m_eRangeType[i]=e_NormDown;
			temp=sscanf(Line,"%1s: from %lf downto %lf step %lf",Letter,&min[i],&max[i],&step[i]);
			if (temp==3) 
				step[i]=-1;
			if (temp < 3)
				temp = 0;
		}
		if (temp == 0)
		{
			temp=sscanf(Line,"%1s: from %lf to %lf step %lf",Letter,&min[i],&max[i],&step[i]);
			m_eRangeType[i]=e_Norm;
			if (temp<4) 
				step[i]=1;
		}
		if (temp<3)
		{
			temp=sscanf(Line,"%1s: primes from %lf to %lf",Letter,&min[i],&max[i]);

			if (temp<3)
			{
				temp=sscanf(Line,"%1s: in { %s ",Letter,s_array[i]);
				if (temp<2)
				{
					delete[] tmpLineBuf;
					throw "Invalid range information in ABC2 file";
				}
				m_eRangeType[i]=e_In;
				tempPtr=strchr(Line,'{')+1;
				while (tempPtr[0]==' ')
					tempPtr++;
				for (j=0;j<1000;j++)
				{
					if (sscanf(tempPtr,"%s",tmpLineBuf)!=1) 
						break;
					if (!strcmp(tmpLineBuf, "}"))
						break;

					// Increase the size a "holdset" value can be
					HoldSet[j] = new char[strlen(tmpLineBuf)+1];
					strcpy(HoldSet[j], tmpLineBuf);

					// fix bug where there is no "trailing" space in the "set" before the closing } 
					if (strchr(HoldSet[j], '}'))
					{
						strtok(HoldSet[j], " }");
						j++;
						break;
					}
					tempPtr=strchr(tempPtr,' ');
					if (!tempPtr)
						break;
					tempPtr++;
				}
				m_pSet[i]=(char **)malloc(sizeof(char *)*j);
				for (k=0;k<j;k++)
				{
					m_pSet[i][k]= new char[strlen(HoldSet[k])+1];
					strcpy(m_pSet[i][k],HoldSet[k]);
					delete[] HoldSet[k];
					HoldSet[k] = 0;
				}
				max[i]=j-1;
				m_nSetNum[i]=0;
			}
			else
			{
				m_eRangeType[i]=e_Prime;
				
				if (m_nFirstPrime==-1)
					m_nFirstPrime=i;

				ABCprimeserver.restart();
				ABCprimeserver.skip(uint64(min[i]));
				uint64 p;
				ABCprimeserver.next(p);
				array[i]=(double)(int64)p;
				if (i==0 && array[i] > 3)
					array[i]--;
			}
		}
		else 
			array[i]=min[i];
		
		if (LetterNumber(Letter[0])!=i)
		{
			delete[] tmpLineBuf;
			throw "Lines in wrong order in ABC2 file";
		}
	}
	delete[] tmpLineBuf;
	array[0]-=step[0];
	if (m_eRangeType[0]==e_In) {
		m_nSetNum[0]=-1;
	}
}

int PFABC2File::GetNextLine(PFString &sLine, Integer * /*pInt*/, bool *pbIntValid, PFSymbolTable *)
{
	int i;
	
SkipThisLine:;

	// we do not return an Integer in the pInt pointer
	if (pbIntValid)
		*pbIntValid = false;
	if (m_bEOF)
	{
		m_sCurrentExpression = "";
		return e_eof;
	}
	bool bStoreThisExpression=false;
	
	if (m_nCurrentMultiPrime==0)
	{
		i=0;
		bool again;
		bStoreThisExpression=true;
		m_nCurrentPrimeCount=0;
		do
		{
			again=false;
            if (m_eRangeType[i]==e_Prime)
			{
                ABCprimeserver.restart();
                ABCprimeserver.skip(uint64(array[i])+1);
				uint64 p;
                ABCprimeserver.next(p);
				array[i]=(double)(int64)p;
                if (array[i]>max[i])
				{
                    ABCprimeserver.restart();
                    ABCprimeserver.skip(uint64(min[i]));
					ABCprimeserver.next(p);
                    array[i]=(double)(int64)p;
                    i++;
                    again=true;
                }
            }
			else if (m_eRangeType[i]==e_In)
			{
				if (m_nSetNum[i]==max[i])
				{
					//array[i]=m_pSet[i][m_nSetNum[i]=0];
					strcpy(s_array[i],m_pSet[i][m_nSetNum[i]=0]);
					i++;
					again=true;
				}
				else
				{
					//array[i]=m_pSet[i][++m_nSetNum[i]];
					strcpy(s_array[i],m_pSet[i][++m_nSetNum[i]]);
				}
			}
			else
			{
				if (m_eRangeType[i]==e_NormDown)
				{
					if ((array[i]+=step[i])<max[i])
					{
						array[i]=min[i];
						i++;
						again=true;
					}
				}
				else
				{
					if ((array[i]+=step[i])>max[i])
					{
						array[i]=min[i];
						i++;
						again=true;
					}
				}
			}
			if (i>m_nLastLetter)
			{
				if (m_pIni)
					m_pIni->SetFileProcessing(false);
				m_bEOF = true;
				m_sCurrentExpression = "";
				return e_eof;
			}
		}
		while (again);

		if (bStoreThisExpression)
		{
			m_nCurrentLineNum++;
			if (m_pIni)
				m_pIni->SetFileLineNum(m_nCurrentLineNum);
		}
	}

	for (i=0;i<=m_nLastLetter;i++)
	{
		if (m_eRangeType[i]!=e_In)
		{
			delete[] s_array[i];	// Fix a memory leak.
			s_array[i] = new char[40];
			sprintf(s_array[i],"%.0f",array[i]);
		}
	}

	if (!m_nCurrentMultiPrime && !ProcessThisLine())
		goto SkipThisLine;

	MakeExpr(sLine);
	if (bStoreThisExpression)
		m_sCurrentExpression = sLine;

	LoadModularFactorString();

	return e_ok;
}


int PFABC2File::SeekToLine(int LineNumber)
{
	double entries[26];
	int i,j;
	PFString sLine;
//	bool goFast=true;

	m_nCurrentLineNum=LineNumber;
	if (m_pIni) {
		m_pIni->SetFileLineNum(LineNumber);
		m_pIni->SetFileProcessing(true);
	}
	m_bEOF = false;

	if (m_eRangeType[0]==e_Norm) {
		entries[0]=int(floor((max[0]-min[0])/step[0]))+1;
		array[0]=min[0]-step[0];
	} else if (m_eRangeType[0]==e_NormDown) {
		entries[0]=int(floor((min[0]-max[0])/(-1*step[0])))+1;
		array[0]=min[0]-step[0];
	} else if (m_eRangeType[0]==e_In) {
		entries[0]=max[0]+1;
		array[0]=-1;
	} else if (m_eRangeType[0]==e_Prime) {
		ABCprimeserver.restart();
		ABCprimeserver.skip(uint64(min[0]));
		j=0;
		array[0]=-1;
		uint64 p;
		for (ABCprimeserver.next(p); p <= (uint64)max[0]; ABCprimeserver.next(p))
			j++;
		entries[0]=j;
	}

	for (i=1;i<=m_nLastLetter;i++) {
		if (m_eRangeType[i]==e_Norm) {
			entries[i]=int(floor((max[i]-min[i])/step[i]))+1;
			array[i]=min[i];
		} else if (m_eRangeType[i]==e_NormDown) {
			entries[i]=int(floor((min[i]-max[i])/(-1*step[i])))+1;
			array[i]=min[i]-step[i];
		} else if (m_eRangeType[i]==e_In) {
			entries[i]=max[i]+1;
			array[i]=0;
		} else if (m_eRangeType[i]==e_Prime) {
			ABCprimeserver.restart();
			ABCprimeserver.skip(uint64(min[i]));
			j=0;
			array[i]=0;
			uint64 p;
			for (ABCprimeserver.next(p); p <= (uint64)max[i]; ABCprimeserver.next(p))
				j++;
			entries[i]=j;
		}
		entries[i]*=entries[i-1];
	}

	LineNumber--;

	while (LineNumber>0) {
		for (i=0;i<=m_nLastLetter && entries[i]<=LineNumber;i++);
		
		if (i>m_nLastLetter) {
			if (m_pIni) 
				m_pIni->SetFileProcessing(false);
			m_bEOF = true;
			return e_eof;
		}

		if (i==0) {
			array[0]+=step[0]*LineNumber;
			LineNumber=0;
		} else {
			array[i]+=step[i]*int(floor(LineNumber/entries[i-1]));
			LineNumber%=uint64(entries[i-1]);
		}
		i--;
	}

	for (i=0;i<=m_nLastLetter;i++) {
		if (m_eRangeType[i]==e_In) {
			m_nSetNum[i]=int(array[i]);
			//array[i]=m_pSet[i][int(array[i])];
			strcpy(s_array[i],m_pSet[i][int(array[i])]);
		}
		else if (m_eRangeType[i]==e_Prime) {
			ABCprimeserver.restart();
			ABCprimeserver.skip(uint64(min[i]));
			uint32 p32;
			uint64 p;
			for (j=0;j<array[i];j++)
				ABCprimeserver.next(p32);
			if (i == 0 && array[i] == -1)
			{
				ABCprimeserver.next(p);
				array[i]=(double)(int64)(p-1);
			}
			else
			{
				ABCprimeserver.next(p);
				array[i]=(double)(int64)(p);
			}
		}
	}

	return e_ok;
}
