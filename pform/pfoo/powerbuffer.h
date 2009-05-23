#ifdef USE_GALLOT
class PowerBuffer : public BufferReciprocal
{
public:
	PowerBuffer();
	~PowerBuffer();
};
#else
class PowerBuffer
{
public:
	PowerBuffer();
	~PowerBuffer();
};
#endif
