class FactorNode;

class FactorArray
{
	FactorNode **fA;
	int nxtFree;
	int nxtAvail;
public:
	FactorArray();
	~FactorArray();
	
	FactorArray(const FactorArray &fa);
	FactorArray &operator=(const FactorArray &fa);

	FactorNode *&operator[](int);
	void add(FactorNode *,const PFBoolean &);

	int heapsize();
	FactorNode *remove(const PFBoolean &);

    // append doesn't sort. makeHeap fixes em up
	void append(FactorNode *);
   void makeHeap(const PFBoolean &);
};
