#include <stddef.h>

void *xmalloc(size_t sz); 
void *xrealloc(void *p, size_t sz);  

#define new(p) ((p) = xmalloc(sizeof(*(p))))
 
