/*
 * lib/netrom_gr.c	This file contains an implementation of the NET/ROM
 *			route support functions.
 *
 * Version:	lib/netrom_gr.c 0.01 (1996-02-15)
 *
 * Author:	Bernd Eckenfels, <ecki@lina.inka.de>
 *		Copyright 1999 Bernd Eckenfels, Germany
 *		base on Code from Jonathan Naylor <jsn@Cs.Nott.AC.UK>
 *
 *		This program is free software; you can redistribute it
 *		and/or  modify it under  the terms of  the GNU General
 *		Public  License as  published  by  the  Free  Software
 *		Foundation;  either  version 2 of the License, or  (at
 *		your option) any later version.
 */
#include "config.h"

#if HAVE_AFNETROM
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include "net-support.h"
#include "pathnames.h"
#define  EXTERN
#include "net-locale.h"

/* UGLY */

int NETROM_rprint(int options)
{
	FILE *f1=fopen(_PATH_PROCNET_NR_NODES, "r");
	FILE *f2=fopen(_PATH_PROCNET_NR_NEIGH, "r");
	char buffer[256];
	int qual,n,w;
	/*int ext = options & FLAG_EXT;
	int numeric = options & FLAG_NUM;*/

	if(f1==NULL||f2==NULL)
	{
		printf(NLS_CATGETS(catfd, netstatSet, netstat_nonetrom, "NET/ROM not configured in this system.\n")); /* xxx */
		return 1;
	}
	printf(NLS_CATGETS(catfd, netstatSet, netstat_netrom, "Kernel NET/ROM routing table\n")); /* xxx */
	printf(NLS_CATGETS(catfd, netstatSet, netstat_header_netrom, "Destination  Mnemonic  Quality  Neighbour  Iface\n")); /* xxx */
	fgets(buffer,256,f1);
	while(fgets(buffer,256,f1))
	{
		buffer[9]=0;
		buffer[17]=0;
		w=atoi(buffer+19)-1;
		printf("%-9s    %-7s   ",
			buffer,buffer+10);
		qual=atoi(buffer+24+15*w);
		n=atoi(buffer+32+15*w);
		rewind(f2);
		fgets(buffer,256,f2);
		while(fgets(buffer,256,f2))
		{
			if(atoi(buffer)==n)
			{
				buffer[15]=0;
				buffer[20]=0;
				printf("%3d      %-9s  %s\n",
					qual,buffer+6,buffer+16);
				break;
			}
		}
	}
	fclose(f1);
	fclose(f2);
	return 0;
}

#endif	/* HAVE_AFNETROM */
