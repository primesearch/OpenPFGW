// GMP "custom" allocation functions.  These functions implement "granular" allocation strategy.

#if defined (COMPUTE_ALLOCATION_COUNTS)
int AllocCnt=0;
int ReallocCnt1=0;
int ReallocCnt2=0;
int FreeCnt=0;
#endif

void *GMP_Custom_alloc_func (size_t size)
{
#if defined (COMPUTE_ALLOCATION_COUNTS)
	AllocCnt++;
#endif
	if (size < 256)
		size = 256;
	else
		size =  ((size>>12)<<12)+(1<<12);	// give the data larger than 256 bytes a 4k granularity.
	return malloc(size);
}

void *GMP_Custom_realloc_func (void *ptr, size_t old_size, size_t new_size)
{
#if defined (COMPUTE_ALLOCATION_COUNTS)
	ReallocCnt1++;
#endif
	if (old_size < 256)
		old_size = 256;
	else
		old_size =  ((old_size>>12)<<12)+(1<<12);// give the data larger than 256 bytes a 4k granularity.

	// no need to realloc any memory, the realloc request is satisfied by the current memory allocation
	if (old_size > new_size)
		return ptr;

	// Ok, we have to actually realloc memory.
#if defined (COMPUTE_ALLOCATION_COUNTS)
	ReallocCnt2++;
#endif

	if (new_size < 256)
		new_size = 256;
	else
		new_size =  ((new_size>>12)<<12)+(1<<12);	// give the data larger than 256 bytes a 4k granularity.
	void *p = realloc(ptr, new_size);
	if (!p)
	{
		fprintf (stderr, "Error allocating memory within GMP routines!\n");
		exit(-1);
	}
	return p;
}

void GMP_Custom_free_func (void *ptr, size_t)
{
#if defined (COMPUTE_ALLOCATION_COUNTS)
	FreeCnt++;
#endif
	free(ptr);
}


