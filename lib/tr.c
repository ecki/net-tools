/*
 * lib/tr.c   This file contains an implementation of the "Tokenring"
 *              support functions.
 *
 * Version:     $Id: tr.c,v 1.9 2005/05/16 03:15:12 ecki Exp $
 *
 * Author:      Fred N. van Kempen, <waltje@uwalt.nl.mugnet.org>
 *              Copyright 1993 MicroWalt Corporation
 *
 *              This program is free software; you can redistribute it
 *              and/or  modify it under  the terms of  the GNU General
 *              Public  License as  published  by  the  Free  Software
 *              Foundation;  either  version 2 of the License, or  (at
 *              your option) any later version.
 */
#include "config.h"

#if HAVE_HWTR
#include <asm/types.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <net/if_arp.h>
#include <linux/if_tr.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <ctype.h>
#include <string.h>
#include <unistd.h>
#include "net-support.h"
#include "pathnames.h"
#include "intl.h"
#include "util.h"


/* actual definition at the end of file */
extern struct hwtype tr_hwtype;
#ifdef ARPHRD_IEEE802_TR
extern struct hwtype tr_hwtype1;
#endif

static const char *pr_tr(const char *ptr)
{
    static char buff[64];

    snprintf(buff, sizeof(buff), "%02X:%02X:%02X:%02X:%02X:%02X",
	     (ptr[0] & 0377), (ptr[1] & 0377), (ptr[2] & 0377),
	     (ptr[3] & 0377), (ptr[4] & 0377), (ptr[5] & 0377)
	);
    return (buff);
      }

#ifdef DEBUG
#define _DEBUG 1
#else
#define _DEBUG 0
#endif

static int in_tr(char *bufp, struct sockaddr_storage *sasp)
{
    struct sockaddr *sap = (struct sockaddr *)sasp;
    char *ptr;
    char c, *orig;
    int i, val;

#ifdef ARPHRD_IEEE802_TR
    if (kernel_version() < KRELEASE(2,3,30)) {
        sap->sa_family = tr_hwtype.type;
    } else {
        sap->sa_family = tr_hwtype1.type;
    }
#else
    sap->sa_family = tr_hwtype.type;
    #warning "Limited functionality, no support for ARPHRD_IEEE802_TR (old kernel headers?)"
#endif

    ptr = sap->sa_data;

    i = 0;
    orig = bufp;
    while ((*bufp != '\0') && (i < TR_ALEN)) {
	val = 0;
	c = *bufp++;
	if (isdigit(c))
	    val = c - '0';
	else if (c >= 'a' && c <= 'f')
	    val = c - 'a' + 10;
	else if (c >= 'A' && c <= 'F')
	    val = c - 'A' + 10;
	else {
	    if (_DEBUG)
		fprintf(stderr, _("in_tr(%s): invalid token ring address!\n"), orig);
	    errno = EINVAL;
	    return (-1);
	}
	val <<= 4;
	c = *bufp++;
	if (isdigit(c))
	    val |= c - '0';
	else if (c >= 'a' && c <= 'f')
	    val |= c - 'a' + 10;
	else if (c >= 'A' && c <= 'F')
	    val |= c - 'A' + 10;
	else {
	    if (_DEBUG)
		fprintf(stderr, _("in_tr(%s): invalid token ring address!\n"), orig);
	    errno = EINVAL;
	    return (-1);
	}
	*ptr++ = (unsigned char) (val & 0377);
	i++;

	/* We might get a semicolon here - not required. */
	if (*bufp == ':') {
	    if (_DEBUG && i == TR_ALEN)
		fprintf(stderr, _("in_tr(%s): trailing : ignored!\n"),
			orig);
	    bufp++;
	}
    }

    /* That's it.  Any trailing junk? */
    if (_DEBUG && (i == TR_ALEN) && (*bufp != '\0')) {
	fprintf(stderr, _("in_tr(%s): trailing junk!\n"), orig);
	errno = EINVAL;
	return (-1);
    }
    if (_DEBUG)
	fprintf(stderr, "in_tr(%s): %s\n", orig, pr_tr(sap->sa_data));

    return (0);
}


struct hwtype tr_hwtype =
{
    "tr", NULL /* "16/4 Mbps Token Ring" */, ARPHRD_IEEE802, TR_ALEN,
    pr_tr, in_tr, NULL
};
#ifdef ARPHRD_IEEE802_TR
struct hwtype tr_hwtype1 =
{
    "tr", NULL /* "16/4 Mbps Token Ring" */, ARPHRD_IEEE802_TR, TR_ALEN,
    pr_tr, in_tr, NULL
};
#endif


#endif				/* HAVE_HWTR */
