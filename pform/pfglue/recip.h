//=======================================================
// Reciprocals and other such stuff
//=======================================================
#ifndef RECIP_H

#ifdef USE_GALLOT

class Reciprocal
{
protected:
	virtual void divmodcore(Integer &x,Integer &y)=0;
public:
	virtual ~Reciprocal()=0;

	void div(Integer &x);
	void mod(Integer &y);
	void divmod(Integer &x,Integer &y);
};

class BufferReciprocal : public Reciprocal
{
	unsigned int q1;		// first shift count
	unsigned int q2;		// second shift count
	
	Integer N;	// the number
	Integer R;
	
	Complex *bmul;
	unsigned int bsmul;	// buffered FFT of the multiplier

	Complex *bdiv;
	unsigned int bsdiv;	// buffered FFT of the divider
protected:
	virtual void divmodcore(Integer &x,Integer &y);
	void eraseBuffers();
	void calculateReciprocal();
	void calculateBuffers();
public:
	BufferReciprocal(const Integer &N);
	~BufferReciprocal();
	
	BufferReciprocal(const BufferReciprocal &b);
	BufferReciprocal &operator=(const BufferReciprocal &b);

	void set(const Integer &X);
	Integer &value() {return N;}
};
#endif

#define RECIP_H
#endif
