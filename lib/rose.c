/*
 * lib/rose.c This file contains an implementation of the "ROSE"
 *              support functions for the NET-2 base distribution.
 *
 * Version:     $Id: rose.c,v 1.5 1998/11/15 20:12:00 freitag Exp $
 *
 * Author:      Terry Dawson, VK2KTJ, <terry@perf.no.itg.telstra.com.au>
 *              based on ax25.c by:
 *              Fred N. van Kempen, <waltje@uwalt.nl.mugnet.org>
 *              Copyright 1993 MicroWalt Corporation
 *
 *              This program is free software; you can redistribute it
 *              and/or  modify it under  the terms of  the GNU General
 *              Public  License as  published  by  the  Free  Software
 *              Foundation;  either  version 2 of the License, or  (at
 *              your option) any later version.
 */
#include "config.h"

#if HAVE_AFROSE || HAVE_HWROSE
#include <features.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <net/if_arp.h>		/* ARPHRD_ROSE */
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include "net-support.h"
#include "pathnames.h"
#include "intl.h"

#if __GLIBC__ >= 2
#include <netrose/rose.h>
#endif

static char ROSE_errmsg[128];

extern struct aftype rose_aftype;

static char *
 ROSE_print(unsigned char *ptr)
{
    static char buff[12];

    snprintf(buff, sizeof(buff), "%02x%02x%02x%02x%02x", ptr[0], ptr[1], ptr[2], ptr[3], ptr[4]);
    buff[10] = '\0';
    return (buff);
}

/* Display a ROSE socket address. */
static char *
 ROSE_sprint(struct sockaddr *sap, int numeric)
{
    if (sap->sa_family == 0xFFFF || sap->sa_family == 0)
	return _("[NONE SET]");

    return (ROSE_print(((struct sockaddr_rose *) sap)->srose_addr.rose_addr));
}


static int ROSE_input(int type, char *bufp, struct sockaddr *sap)
{
    char *ptr;
    int i, o;

    sap->sa_family = rose_aftype.af;
    ptr = ((struct sockaddr_rose *) sap)->srose_addr.rose_addr;

    /* Node address the correct length ? */
    if (strlen(bufp) != 10) {
	strcpy(ROSE_errmsg, _("Node address must be ten digits"));
#ifdef DEBUG
	fprintf(stderr, "rose_input(%s): %s !\n", ROSE_errmsg, orig);
#endif
	errno = EINVAL;
	return (-1);
    }
    /* Ok, lets set it */
    for (i = 0, o = 0; i < 5; i++) {
	o = i * 2;
	ptr[i] = (((bufp[o] - '0') << 4) | (bufp[o + 1] - '0'));
    }

    /* All done. */
#ifdef DEBUG
    fprintf(stderr, "rose_input(%s): ", orig);
    for (i = 0; i < sizeof(rose_address); i++)
	fprintf(stderr, "%02X ", sap->sa_data[i] & 0377);
    fprintf(stderr, "\n");
#endif

    return (0);
}


/* Display an error message. */
static void ROSE_herror(char *text)
{
    if (text == NULL)
	fprintf(stderr, "%s\n", ROSE_errmsg);
    else
	fprintf(stderr, "%s: %s\n", text, ROSE_errmsg);
}


static char *
 ROSE_hprint(struct sockaddr *sap)
{
    if (sap->sa_family == 0xFFFF || sap->sa_family == 0)
	return _("[NONE SET]");

    return (ROSE_print(((struct sockaddr_rose *) sap)->srose_addr.rose_addr));
}


static int ROSE_hinput(char *bufp, struct sockaddr *sap)
{
    if (ROSE_input(0, bufp, sap) < 0)
	return (-1);
    sap->sa_family = ARPHRD_ROSE;
    return (0);
}

struct hwtype rose_hwtype =
{
    "rose", NULL, /*"AMPR ROSE", */ ARPHRD_ROSE, 10,
    ROSE_print, ROSE_hprint, ROSE_hinput, NULL
};

struct aftype rose_aftype =
{
    "rose", NULL, /*"AMPR ROSE", */ AF_ROSE, 10,
    ROSE_print, ROSE_sprint, ROSE_input, ROSE_herror,
    NULL, NULL, NULL,
    -1,
    "/proc/net/rose"
};

#endif				/* HAVE_xxROSE */
