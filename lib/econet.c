/*
 * lib/econet.c This file contains an implementation of the Econet
 *              support functions for the net-tools.
 *
 * Author:      Philip Blundell <philb@gnu.org>
 *
 *              This program is free software; you can redistribute it
 *              and/or  modify it under  the terms of  the GNU General
 *              Public  License as  published  by  the  Free  Software
 *              Foundation;  either  version 2 of the License, or  (at
 *              your option) any later version.
 */

#include "config.h"

#if HAVE_AFECONET

#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <neteconet/ec.h>

#include "version.h"
#include "net-support.h"
#include "pathnames.h"
#include "intl.h"


/* Display an Econet address */
static const char *
ec_print(const char *ptr)
{
    static char buff[64];
    struct ec_addr *ec = (struct ec_addr *) ptr;
    sprintf(buff, "%d.%d", ec->net, ec->station);
    return buff;
}


/* Display an Econet socket address */
static const char *
ec_sprint(const struct sockaddr_storage *sasp, int numeric)
{
    const struct sockaddr_ec *sec = (const struct sockaddr_ec *)sasp;

    if (sasp->ss_family != AF_ECONET)
	return _("[NONE SET]");

    return ec_print((const char *) &sec->addr);
}

static int
ec_input(int type, char *bufp, struct sockaddr_storage *sasp)
{
    struct sockaddr_ec *sec = (struct sockaddr_ec *) sasp;
    int net, stn;
    switch (sscanf(bufp, "%d.%d", &net, &stn)) {
    case 2:
	sec->addr.station = stn;
	sec->addr.net = net;
	return 0;
    case 1:
	if (sscanf(bufp, "%d", &stn) == 1) {
	    sec->addr.net = 0;
	    sec->addr.station = stn;
	    return 0;
	}
    }
    return -1;
}

struct aftype ec_aftype =
{
    "ec", NULL, AF_ECONET, 0,
    ec_print, ec_sprint, ec_input, NULL,
    NULL, NULL, NULL,
    -1,
    "/proc/sys/net/econet"
};

#endif				/* HAVE_AFECONET */
