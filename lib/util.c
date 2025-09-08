/*
 * lib/util.c net-tools utilities
 * Copyright 1998 by Andi Kleen. Subject to the GPL.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/utsname.h>
#include <unistd.h>

#include "util.h"


static void oom(void)
{
    fprintf(stderr, "out of virtual memory\n");
    exit(2);
}

void *xmalloc(size_t sz)
{
    void *p = calloc(sz, 1);
    if (!p)
	oom();
    return p;
}

/* Like strdup, but oom() instead of NULL */
char *xstrdup(const char *s)
{
    char *d = strdup(s);
    if (!d)
        oom();
    return d;
}

void *xrealloc(void *oldp, size_t sz)
{
    void *p = realloc(oldp, sz);
    if (!p)
	oom();
    return p;
}

int kernel_version(void)
{
    struct utsname uts;
    int major, minor, patch=0;

    if (uname(&uts) < 0)
	return -1;
    if (sscanf(uts.release, "%d.%d.%d", &major, &minor, &patch) < 2)
	return -1;
    return KRELEASE(major, minor, patch);
}

long ticks_per_second(void)
{
    return sysconf(_SC_CLK_TCK);
}

/* Like strncpy but make sure the resulting string is always 0 terminated. */
char *safe_strncpy(char *dst, const char *src, size_t size)
{
    dst[size-1] = '\0';
    return strncpy(dst,src,size-1);
}
