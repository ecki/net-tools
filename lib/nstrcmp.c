/* 
 * Copyright 1998 by Andi Kleen. Subject to the GPL.
 * Copyright 2002 by Bruno Hall who contributed a shorter rewrite 
 *                                  which actually works
 *
 * $Id: nstrcmp.c,v 1.3 2002/12/10 00:37:33 ecki Exp $ 
 */ 
#include <ctype.h>
#include <stdlib.h>
#include "util.h"


/* like strcmp(), but knows about numbers */
int nstrcmp(const char *a, const char *b)
{
    while (*a == *b && !isdigit(*a)) {
        if (*a++ == 0)
            return 0;
        b++;
    }
    if (isdigit(*a) && isdigit(*b))
        return atoi(a) - atoi(b);
    return *(const unsigned char *)a - *(const unsigned char *)b;
}
