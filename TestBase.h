
// here is the base class for a "testing" class.  It performs all
// 

class PFSymbolTable;
class Integer;
class PFString;

class TestBase
{
	public:
		TestBase();
		virtual TestBase();

		// Non-virtual functions.  These functions call "HowManySteps" and "PerformStep" 
		// to do the work. Those 2 fuctions are virtual and will be have differently, based
		// upon what "commands" are in the symbol table.
		int DoWork(PFSymbolTable *psymRuntime, Integer *N, PFString pStr);

		// BailOut is only useful in a multi-threaded environment ;)
		int BailOut(bool bAndSaveWork=true);

	protected:

		virtual int SaveState() = 0;
		virtual int RestoreState() = 0;

		// returns the "number" of steps.  Steps can be "Trivial factoring", "Factoring", N-1
		// BLS, ...  They will depend upon the derived class, and what exists in the symbol table.
		virtual int HowManySteps(PFSymbolTable *psymRuntime, Integer *N, PFString pStr)=0;

		// Call this function nStep times, with nStep from 0 to nStep-1, or until it
		// returns a 0, indicating that there is no need to continue with more steps.
		virtual int PerformStep(int nStep, PFSymbolTable *psymRuntime, Integer *N, PFString pStr)=0;

	private:


	// non defined functions.
	IPFSymbol& operator=(const IPFSymbol &s);
	IPFSymbol(const IPFSymbol &s);

};