/*
 * lib/ash.c	This file contains an implementation of the Ash
 *		support functions for the NET-2 base distribution.
 */

#include "config.h"

#if HAVE_HWASH

#include <sys/types.h>
#include <sys/socket.h>
#include <net/if_arp.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <ctype.h>
#include <string.h>
#include <unistd.h>
#include "net-support.h"
#include "pathnames.h"
#define  EXTERN
#include "net-locale.h"

#ifndef ARPHRD_ASH
#error Your C library does not support Ash
#endif

#define ASH_ALEN		32

extern struct hwtype ash_hwtype;

/* Display an Ash address in readable format. */
static char *
pr_ash(unsigned char *ptr)
{
  static char buff[128];
  char *p = buff;
  unsigned int i = 0;

  while (ptr[i] != 0xc9 && (i < ASH_ALEN)) {
	  sprintf(p, "%x:", ptr[i]);
	  i++;
	  p += 2;
  }
  
  if (p != buff) 
	  p[-1] = 0;
  else
	  p[0] = 0;
  return buff;
}

/* Display an Ash socket address. */
static char *
pr_sash(struct sockaddr *sap)
{
  static char buf[64];

  if (sap->sa_family == 0xFFFF || sap->sa_family == 0)
    return strncpy(buf, "[NONE SET]", 64);
  return(pr_ash(sap->sa_data));
}


static int
in_ash(char *bufp, struct sockaddr *sap)
{
  unsigned char *ptr;
  unsigned int i = 0;

  sap->sa_family = ash_hwtype.type;
  ptr = sap->sa_data;

  while (bufp && i < 32) {
	  char *next;
	  int hop = strtol(bufp, &next, 16);
	  ptr[i++] = hop;
	  switch (*next) {
	  case ':':
		  bufp = next + 1;
		  break;
	  case 0:
		  bufp = NULL;
		  break;
	  default:
		  fprintf(stderr, "Malformed Ash address");
		  memset(ptr, 0xc9, 32);
		  return -1;
	  }
  }

  while (i < 32)
	  ptr[i++] = 0xc9;

  return 0;
}


struct hwtype ash_hwtype = {
  "ash",	NULL, 		ARPHRD_ASH,	ASH_ALEN,
  pr_ash,	pr_sash,	in_ash,		NULL
};

#endif
