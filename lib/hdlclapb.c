/*
 * lib/hdlclapb.c 
 *		This file contains the HDLC/LAPB support for the NET-2 base
 *		distribution.
 *
 * Version:	@(#)hdlclapb.c	0.10	22/04/1998
 *
 * Original Author:	
 *		Fred N. van Kempen, <waltje@uwalt.nl.mugnet.org>
 *		Copyright 1993 MicroWalt Corporation
 *
 *		Modified by Alan Cox, May 94 to cover NET-3
 *
 *		This program is free software; you can redistribute it
 *		and/or  modify it under  the terms of  the GNU General
 *		Public  License as  published  by  the  Free  Software
 *		Foundation;  either  version 2 of the License, or  (at
 *		your option) any later version.
 */
#include "config.h"

#if HAVE_HWHDLCLAPB

#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <net/if_arp.h>
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

struct hwtype hdlc_hwtype = {
  "hdlc",	NULL, /*"(Cisco) HDLC",*/	ARPHRD_HDLC,	0,
  NULL,		NULL,		NULL,		NULL,
};
struct hwtype lapb_hwtype = {
  "lapb",	NULL, /*"LAPB",*/		ARPHRD_LAPB,	0,
  NULL,		NULL,		NULL,		NULL,
};

#endif	/* HAVE_HWHDLCLAPB */
