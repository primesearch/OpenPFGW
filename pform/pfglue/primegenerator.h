#ifndef PRIMEGENERATOR_H
#define PRIMEGENERATOR_H

struct _primegen;

class PrimeGenerator
{
	struct _primegen *pg;
public:
	PrimeGenerator();
	virtual ~PrimeGenerator();
	
	void restart();
	void next(uint64 &p);
	void next(uint32 &p);
	void skip(uint32 to);
	void skip(uint64 to);
private:
	PrimeGenerator(const PrimeGenerator &);
	PrimeGenerator &operator=(const PrimeGenerator &);
};

#endif
