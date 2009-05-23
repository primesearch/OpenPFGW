#ifndef PFFUNCTIONSYMBOL_H
#define PFFUNCTIONSYMBOL_H

class PFFunctionSymbol : public IPFSymbol
{
public:
	PFFunctionSymbol(const PFString &sName);
	
	virtual DWORD MinimumArguments() const=0;
	virtual DWORD MaximumArguments() const=0;
	virtual DWORD GetArgumentType(DWORD dw) const=0;
	virtual PFString GetArgumentName(DWORD dw) const=0;
	virtual PFBoolean CallFunction(PFSymbolTable *pContext)=0;
	
	PFString GetStringValue();
	DWORD GetSymbolType() const;
	
	virtual void ClearPersistentData();
	
	static int CallSubroutine(const PFString &sRoutineName,PFSymbolTable *pContext);
	static void LoadExprFunctions(PFSymbolTable *pContext);
	static void LoadAllFunctions(PFSymbolTable *pContext);
};
#endif
