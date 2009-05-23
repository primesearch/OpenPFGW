/*================== PrimeForm (c) 1999-2000 ===========================
 * File: pfqueue.h
 *
 * Description:
 * Template class for a queue
 *======================================================================
 */

template<class T>
class PFQueue : private PFList<T>
{
public:
	PFQueue(const PFBoolean &bOwns=PFBoolean::b_false);
	virtual ~PFQueue();

	void Queue(const T* pObject const);
	T* Dequeue();
};

template<class T>
PFQueue<T>::PFQueue(const PFBoolean& bOwns)
	: PFList<T>(bOwns)
{
};

template<class T>
PFQueue<T>::~PFQueue()
{
}

template<class T>
void PFQueue<T>::Queue(const T* pObject const)
{
	AddTail(pObject);
}

template<class T>
T* PFStack<T>::Dequeue()
{
	return RemoveHead(pObject);
}
