/*
 * lib/sit.c	This file contains the SIT HW-type support.
 *
 * Version:	sit.c	0.01 (1996-08-08)
 *
 * Author:	Fred N. van Kempen, <waltje@uwalt.nl.mugnet.org>
 *		Copyright 1993 MicroWalt Corporation
 *
 *		Based on slip.c, modified by Frank Strauss, Aug 1996
 *
 *		This program is free software; you can redistribute it
 *		and/or  modify it under  the terms of  the GNU General
 *		Public  License as  published  by  the  Free  Software
 *		Foundation;  either  version 2 of the License, or  (at
 *		your option) any later version.
 */
#include "config.h"

#if HAVE_HWSIT

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

struct hwtype sit_hwtype = {
  "sit",	NULL, /*"IPv6-in-IPv4",*/		ARPHRD_SIT,	0,
  NULL,		NULL,		NULL,		NULL
};

#endif	/* HAVE_HWSIT */
