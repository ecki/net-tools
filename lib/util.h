#include <stddef.h>

void *xmalloc(size_t sz);
void *xrealloc(void *p, size_t sz);
char *xstrdup(const char *src);

#define new(p) ((p) = xmalloc(sizeof(*(p))))


int kernel_version(void);
#define KRELEASE(maj,min,patch) ((maj) * 10000 + (min)*1000 + (patch))

long ticks_per_second(void);

int nstrcmp(const char *, const char *);

char *safe_strncpy(char *dst, const char *src, size_t size);


#define netmin(a,b) ((a)<(b) ? (a) : (b))
#define netmax(a,b) ((a)>(b) ? (a) : (b))

#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))
