/*
 * ipx_sr.c       This files contains IPX related route manipulation methods.
 *
 * Part of net-tools, the Linux base networking tools
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License.
 */

#include "config.h"

#include <sys/socket.h>
#if AF_IPX
#include <sys/param.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <arpa/nameser.h>
#include <sys/ioctl.h>
#include <ctype.h>
#include <errno.h>
#include <netdb.h>
#include <resolv.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include "version.h"
#include "net-support.h"
#include "pathnames.h"
#include "intl.h"

#include "net-features.h"

extern struct aftype ipx_aftype;

/* static int skfd = -1; */

int IPX_rinput(int action, int ext, char **args)
{

    fprintf(stderr, _("IPX: this needs to be written\n"));
    return (E_NOSUPP);
}
#endif				/* AF_IPX */
