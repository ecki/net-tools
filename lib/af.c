/*
 * lib/af.c	This file contains the top-level part of the protocol
 *		support functions module for the NET-2 base distribution.
 *
 * Version:	lib/af.c 1.13 (1996-02-21)
 *
 * Author:	Fred N. van Kempen, <waltje@uwalt.nl.mugnet.org>
 *		Copyright 1993 MicroWalt Corporation
 *
 *		This program is free software; you can redistribute it
 *		and/or  modify it under  the terms of  the GNU General
 *		Public  License as  published  by  the  Free  Software
 *		Foundation;  either  version 2 of the License, or  (at
 *		your option) any later version.
 */
#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <ctype.h>
#include <string.h>
#include <unistd.h>
#include "config.h"
#include "net-support.h"
#include "pathnames.h"
#define  EXTERN
#include "net-locale.h"


int flag_unx = 0;
int flag_ipx = 0;
int flag_ax25 = 0;
int flag_ddp = 0;
int flag_netrom = 0;
int flag_inet = 0;
int flag_inet6 = 0;
int flag_econet = 0;


struct aftrans_t {
	char *alias;
	char *name;
	int  *flag;
} aftrans[]={
	{"ax25",	"ax25",		&flag_ax25},
	{"ip",		"inet",		&flag_inet},
	{"ip6",		"inet6",       	&flag_inet6},
	{"ipx",		"ipx",		&flag_ipx},
	{"appletalk",	"ddp",		&flag_ddp},
	{"netrom",	"netrom",	&flag_netrom},
	{"inet",	"inet",		&flag_inet},
	{"inet6",	"inet6",       	&flag_inet6},
	{"ddp",		"ddp",		&flag_ddp},
	{"unix",	"unix",		&flag_unx},
	{"tcpip",	"inet",		&flag_inet},
	{"econet",	"ec",		&flag_econet},
	{0,		0,		0}
};

char		afname[256]="";

extern	struct aftype	unspec_aftype;
extern	struct aftype	unix_aftype;
extern	struct aftype	inet_aftype;
extern	struct aftype	inet6_aftype;
extern	struct aftype	ax25_aftype;
extern	struct aftype	netrom_aftype;
extern	struct aftype	ipx_aftype;
extern	struct aftype	ddp_aftype;
extern	struct aftype	ec_aftype;

static short sVafinit = 0;

static struct aftype *aftypes[] = {
#if HAVE_AFUNIX
  &unix_aftype,
#endif
#if HAVE_AFINET
  &inet_aftype,
#endif
#if HAVE_AFINET6
  &inet6_aftype,
#endif
#if HAVE_AFAX25
  &ax25_aftype,
#endif
#if HAVE_AFNETROM
  &netrom_aftype,
#endif
#if HAVE_AFIPX
  &ipx_aftype,
#endif
#if HAVE_AFATALK
  &ddp_aftype,
#endif
#if HAVE_AFECONET
  &ec_aftype,
#endif
  &unspec_aftype,
  NULL
};

void afinit ()
{
  unspec_aftype.title = NLS_CATSAVE (catfd, unixSet, unix_unspec, "UNSPEC");
#if HAVE_AFINET
  unix_aftype.title = NLS_CATSAVE (catfd, unixSet, unix_unix, "UNIX Domain");
#endif
#if HAVE_AFINET
  inet_aftype.title = NLS_CATSAVE (catfd, inetSet, inet_darpa, "DARPA Internet");
#endif
#if HAVE_AFINET6
  inet6_aftype.title = NLS_CATSAVE (catfd, inetSet, inet_darpa, "IPv6");
#endif
#if HAVE_AFAX25
  ax25_aftype.title = NLS_CATSAVE (catfd, ax25Set, ax25_ax25, "AMPR AX.25");
#endif
#if HAVE_AFNETROM
  netrom_aftype.title = NLS_CATSAVE (catfd, netromSet, netrom_netrom, "AMPR NET/ROM");
#endif
#if HAVE_AFIPX
  ipx_aftype.title = NLS_CATSAVE (catfd, ipxSet, ipx_ipx, "IPX");
#endif
#if HAVE_AFATALK
  ddp_aftype.title = NLS_CATSAVE (catfd, ddpSet, ddp_ddp, "Appletalk DDP");
#endif
#if HAVE_AFCONET
  ec_aftype.title = NLS_CATSAVE (catfd, ecSet, ec_ec, "Econet");
#endif
  sVafinit = 1;
}

/* set the default AF list from the program name or a constant value	*/
void
aftrans_def(char *tool, char *argv0, char *dflt)
{
  char *tmp;
  char *buf;
  
  strcpy(afname, dflt);

  if (!(tmp = strrchr(argv0, '/')))
  	tmp = argv0; 			/* no slash?! */
  else
  	tmp++;
  
  if (!(buf = strdup(tmp)))
  	return;

  if (strlen(tool) >= strlen(tmp)) {
  	free(buf);
  	return;
  }
  tmp = buf+(strlen(tmp)-strlen(tool));
  
  if (strcmp(tmp, tool)!=0) {
  	free(buf);
  	return;
  }
  
  *tmp = '\0';
  if ((tmp = strchr(buf,'_')))
  	*tmp = '\0';
  
  afname[0]='\0';
  if (aftrans_opt(buf))
   	strcpy(afname, buf);

  free(buf);
}


/* Check our protocol family table for this family. */
struct aftype *
get_aftype(const char *name)
{
  struct aftype **afp;

  if (!sVafinit)
    afinit ();
  
  afp = aftypes;
  while (*afp != NULL) {
	if (!strcmp((*afp)->name, name)) return(*afp);
	afp++;
  }
  if (index(name,','))
  	fprintf(stderr,NLS_CATGETS(catfd, libSet, lib_toomuch, "Please don't supply more than one address family.\n"));
  return(NULL);
}


/* Check our protocol family table for this family. */
struct aftype *
get_afntype(int af)
{
  struct aftype **afp;

  if (!sVafinit)
    afinit ();
  
  afp = aftypes;
  while (*afp != NULL) {
	if ((*afp)->af == af) return(*afp);
	afp++;
  }
  return(NULL);
}


int aftrans_opt(const char *arg)
{
	struct aftrans_t *paft;
	char *tmp1, *tmp2;
	char buf[256];
		
	strncpy(buf,arg,sizeof(buf));
	buf[sizeof(buf)-1]='\0';
	
	tmp1=buf;
	
	while(tmp1) {
	
		tmp2=index(tmp1,',');

		if (tmp2)
			*(tmp2++)='\0';
			
		paft=aftrans;
		for(paft=aftrans;paft->alias;paft++) {
			if (strcmp(tmp1,paft->alias))
				continue;
			if (strlen(paft->name)+strlen(afname)+1 >= sizeof(afname)) {
				fprintf(stderr,NLS_CATGETS(catfd, libSet, lib_toomuch_af, "Too much address family arguments.\n"));				
				return(0);
			}
			if (paft->flag)
				(*paft->flag)++;
			if (afname[0])
				strcat(afname,",");
			strcat(afname,paft->name);
			break;
		}
		if (!paft->alias) {
			fprintf(stderr,NLS_CATGETS(catfd, libSet, lib_unknown_af, "Unknown address family `%s'.\n"),tmp1);			
			return(1);
		}
		tmp1=tmp2;
	}

	return(0);
}
