#include <stdio.h>
#include <stdlib.h>
#include "util.h"

static void oom(void)
{ 
	fprintf(stderr, "out of virtual memory\n");
	exit(2); 
}

void *xmalloc(size_t sz)
{
	void *p = calloc(sz,1); 
	if (!p) 
		oom();
	return p;
}

void *xrealloc(void *oldp, size_t sz)
{
	void *p = realloc(oldp,sz);
	if (!p) 
		oom();
	return p;
}
