/*
 * lib/econet.c	This file contains an implementation of the Econet
 *		support functions for the net-tools.
 *		(NET-3 base distribution).
 *
 * Version:	lib/econet.c 1.00 1998-04-10
 *
 * Author:	Philip Blundell <philb@gnu.org>
 *
 * Modified:
 *
 *		This program is free software; you can redistribute it
 *		and/or  modify it under  the terms of  the GNU General
 *		Public  License as  published  by  the  Free  Software
 *		Foundation;  either  version 2 of the License, or  (at
 *		your option) any later version.
 */

#include "config.h"

#if HAVE_AFECONET

#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>

#include "version.h"
#include "net-support.h"
#include "pathnames.h"

#define  EXTERN
#include "net-locale.h"

#include <linux/if_ec.h>

/* Display an Econet address */
static char *
ec_print(unsigned char *ptr)
{
  static char buff[64];
  struct ec_addr *ec = (struct ec_addr *)ptr;
  sprintf(buff,"%d.%d", ec->net, ec->station);
  return buff;
}


/* Display an Econet socket address */
static char *
ec_sprint(struct sockaddr *sap, int numeric)
{
  static char buf[64];
  struct sockaddr_ec *sec = (struct sockaddr_ec *)sap;

  if (sap->sa_family != AF_ECONET)
    return(NLS_CATBUFF (catfd, ecSet, ec_none, "[NONE SET]", buf, 64));

  return ec_print((unsigned char *)&sec->addr);
}

static int
ec_input(int type, char *bufp, struct sockaddr *sap)
{
  struct sockaddr_ec *sec = (struct sockaddr_ec *)sap;
  int net, stn;
  switch (sscanf(bufp, "%d.%d", &net, &stn))
  {
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

struct aftype ec_aftype = {
  "ec",	NULL,	AF_ECONET,	0,
  ec_print,	ec_sprint,	ec_input,	NULL,
  NULL
};

#endif /* HAVE_AFECONET */
