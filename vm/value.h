#ifndef value_h
#define value_h

typedef struct value_t
{
	int type;
	union
	{
		int b;
		long i;
		double f;
		void* p;
		void* o;
	} v;
} value_t;

#endif
