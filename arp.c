/*
 * arp		This file contains an implementation of the command
 *		that maintains the kernel's ARP cache.  It is derived
 *		from Berkeley UNIX arp(8), but cleaner and with sup-
 *		port for devices other than Ethernet.
 *
 * NET-TOOLS	A collection of programs that form the base set of the
 *		NET-3 Networking Distribution for the LINUX operating
 *		system.
 *
 * Version:	arp 1.69 (1996-05-17)
 *
 * Maintainer:	Bernd 'eckes' Eckenfels, <net-tools@lina.inka.de>
 *
 * Author: 	Fred N. van Kempen, <waltje@uwalt.nl.mugnet.org>
 *
 * Changes:
 *              Alan Cox        :	modified for NET3
 *              Andrew Tridgell :	proxy arp netmasks
 *              Bernd Eckenfels :	-n option
 *              Bernd Eckenfels :	Use only /proc for display
 *	 {1.60}	Bernd Eckenfels :	new arpcode (-i) for 1.3.42 but works 
 *					with 1.2.x, too
 *	 {1.61}	Bernd Eckenfels :	more verbose messages
 *	 {1.62}	Bernd Eckenfels :	check -t for hw adresses and try to
 *					explain EINVAL (jeff)
 *960125 {1.63}	Bernd Eckenfels :	-a print hardwarename instead of tiltle
 *960201 {1.64} Bernd Eckenfels :	net-features.h support
 *960203 {1.65} Bernd Eckenfels :	"#define" in "#if", 
 *					-H|-A additional to -t|-p
 *960214 {1.66} Bernd Eckenfels :	Fix optarg required for -H and -A
 *960412 {1.67} Bernd Eckenfels :	device=""; is default
 *960514 {1.68}	Bernd Eckenfels :	-N and -D
 *960517 {1.69}	Bernd Eckenfels :	usage() fixed
 *
 *		This program is free software; you can redistribute it
 *		and/or  modify it under  the terms of  the GNU General
 *		Public  License as  published  by  the  Free  Software
 *		Foundation;  either  version 2 of the License, or  (at
 *		your option) any later version.
 */
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <net/if_arp.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <ctype.h>
#include <fcntl.h>
#include <string.h>
#include <getopt.h>
#include <unistd.h>
#include "net-support.h"
#include "pathnames.h"
#include "version.h"
#include "config.h"
#include "net-locale.h"

#define DFLT_AF	"inet"
#define DFLT_HW	"ether"

#define FEATURE_ARP
#include "lib/net-features.h"


char *Release = RELEASE,
     *Version = "arp 1.69 (1996-05-17)";

int opt_n = 0;				/* do not resolve addresses	*/
int opt_N = 0;				/* use symbolic names           */
int opt_v = 0;				/* debugging output flag	*/
int opt_D = 0;				/* HW-address is devicename     */
struct aftype *ap;			/* current address family	*/
struct hwtype *hw;			/* current hardware type	*/
int sockfd=0;				/* active socket descriptor     */
int hw_set = 0;				/* flag if hw-type was set (-H)	*/
char device[16]="";			/* current device		*/
static void usage(void);

/* Delete an entry from the ARP cache. */
static int
arp_del(char **args)
{
  char host[128];
  struct arpreq req;
  struct sockaddr sa;
#if HAVE_NEW_SIOCSARP
  struct arpreq_old old_req;
#endif

  memset((char *) &req, 0, sizeof(req));

  /* Resolve the host name. */
  if (*args == NULL) {
	fprintf(stderr, NLS_CATGETS(catfd, arpSet, arp_hostname, "arp: need host name\n"));
	return(-1);
  }
  strcpy(host, *args);
  if (ap->input(0, host, &sa) < 0) {
	ap->herror(host);
	return(-1);
  }

  /* If a host has more than one address, use the correct one! */
  memcpy((char *) &req.arp_pa, (char *) &sa, sizeof(struct sockaddr));

  req.arp_flags=0;
  
  if (args[1]) {
	if (strcmp(args[1],"pub")==0)
		req.arp_flags|=ATF_PUBL;
	else
		usage();
  }

#if HAVE_NEW_SIOCSARP
  strcpy(req.arp_dev,device);
  memcpy((char *)&old_req,(char *)&req,sizeof(old_req));

  /* Call the kernel. */
  if (opt_v)  fprintf(stderr,"arp: SIOCDARP()\n");
  if (ioctl(sockfd, SIOCDARP, &req) < 0) {
	if (errno == EINVAL) {
		if (opt_v)  fprintf(stderr,"arp: OLD_SIOCDARP()\n");
		if (ioctl(sockfd, OLD_SIOCDARP, &old_req) < 0) {
			if (errno != ENXIO) {
				perror("OLD_SIOCSARP");
				return(-1);
			}
		} else {
			return(0);
		}
	}
	if (errno == ENXIO) {
		printf(NLS_CATGETS(catfd, arpSet, arp_no_arp, 
			"No ARP entry for %s\n"), host);
		return(-1);
	}
	perror("SIOCDARP");
	return(-1);
  }
#else
  /* Call the kernel. */
  if (opt_v)  fprintf(stderr,"arp: old_SIOCDARP()\n");
  if (ioctl(sockfd, SIOCDARP, &req) < 0) {
	perror("SIOCDARP");
	return(-1);
  }
#endif

  return(0);
}

/* Get the hardware address to a specified interface name */
static int
arp_getdevhw(char *ifname, struct sockaddr *sa, struct hwtype *hw)
{
  struct ifreq ifr;
  struct hwtype *xhw;

  strcpy(ifr.ifr_name, ifname);
  if (ioctl(sockfd, SIOCGIFHWADDR, &ifr) < 0) {
      fprintf(stderr,"arp: cant get HW-Address for `%s': %s.\n", ifname, strerror(errno));
      return(-1);
  }
  if (hw && (ifr.ifr_hwaddr.sa_family!=hw->type)) {
      fprintf(stderr,"arp: protocol type missmatch.\n");
      return(-1);
  }
  memcpy((char *)sa, (char *)&(ifr.ifr_hwaddr), sizeof(struct sockaddr));

  if (opt_v) {
      if (!(xhw = get_hwntype(ifr.ifr_hwaddr.sa_family)) || (xhw->sprint==0)) {
        xhw = get_hwntype(-1);
      }
      fprintf(stderr, "arp: device `%s' has HW address %s `%s'.\n",ifname, xhw->name, xhw->sprint(&ifr.ifr_hwaddr));
  }
  return(0);
}

/* Set an entry in the ARP cache. */
static int
arp_set(char **args)
{
  char host[128];
  struct arpreq req;
#if HAVE_NEW_SIOCSARP
  struct arpreq_old old_req;
#endif
  struct sockaddr sa;
  int flags;

  memset((char *) &req, 0, sizeof(req));

  /* Resolve the host name. */
  if (*args == NULL) {
	fprintf(stderr, NLS_CATGETS(catfd, arpSet, arp_hostname, "arp: need host name\n"));
	return(-1);
  }
  strcpy(host, *args++);
  if (ap->input(0, host, &sa) < 0) {
	ap->herror(host);
	return(-1);
  }

  /* If a host has more than one address, use the correct one! */
  memcpy((char *) &req.arp_pa, (char *) &sa, sizeof(struct sockaddr));

  /* Fetch the hardware address. */
  if (*args == NULL) {
	fprintf(stderr, NLS_CATGETS(catfd, arpSet, arp_need_hw, "arp: need hardware address\n"));
	return(-1);
  }

  if (opt_D) {
    if (arp_getdevhw(*args++, &req.arp_ha, hw_set?hw:NULL) < 0)
      return(-1);
  } else {
    if (hw->input(*args++, &req.arp_ha) < 0) {
	fprintf(stderr, NLS_CATGETS(catfd, arpSet, arp_invalidhw, "arp: invalid hardware address\n"));
	return(-1);
    }
  }
  
  /* Check out any modifiers. */
  flags = ATF_PERM;
  while (*args != NULL) {
	if (! strcmp(*args, "temp")) flags &= ~ATF_PERM;
	if (! strcmp(*args, "pub")) flags |= ATF_PUBL;
/*	if (! strcmp(*args, "rarp")) flags |= ATF_RARP;*/
	if (! strcmp(*args, "trail")) flags |= ATF_USETRAILERS;
	if (! strcmp(*args, "netmask")) 
	  {
	    if (*++args == NULL) usage();
	    if (strcmp(*args,"255.255.255.255") != 0)
	      {
		strcpy(host, *args);
		if (ap->input(0, host, &sa) < 0) {
			ap->herror(host);
			return(-1);
		}
		memcpy((char *) &req.arp_netmask, (char *) &sa,
		       sizeof(struct sockaddr));
		flags |= ATF_NETMASK;
	      }
	  }
	args++;
  }

  if ((flags & ATF_NETMASK) && !(flags & ATF_PUBL))
    usage();

  /* Fill in the remainder of the request. */
  req.arp_flags = flags;

#if HAVE_NEW_SIOCSARP
  strcpy(req.arp_dev,device);
  memcpy((char *)&old_req,(char *)&req,sizeof(old_req));

  /* Call the kernel. */
  if (opt_v)  fprintf(stderr,"arp: SIOCSARP()\n");
  if (ioctl(sockfd, SIOCSARP, &req) < 0) {
	if (errno != EINVAL) {
		perror("SIOCSARP");
		return(-1);
	}
	if (opt_v)  fprintf(stderr,"arp: OLD_SIOCSARP()\n");
	if (ioctl(sockfd, OLD_SIOCSARP, &old_req) < 0) {
		if (errno != EINVAL) {
			perror("OLD_SIOCSARP");
			return(-1);
		}
		perror("SIOCSARP and OLD_SIOCSARP");
		if (flags & ATF_PUBL)
			fprintf(stderr, NLS_CATGETS(catfd, arpSet, arp_einv_pub, 
					"Probably destination is reached via ARP Interface. See arp(8)\n"));
		else
			fprintf(stderr, NLS_CATGETS(catfd, arpSet, arp_einv_nopub, 
					"Probably destination is on different Interface. See arp(8)\n"));
		return(-1);
	}
  }
#else
  /* Call the kernel. */
  if (opt_v)  fprintf(stderr,"arp: old_SIOCSARP()\n");
  if (ioctl(sockfd, SIOCSARP, &req) < 0) {
	perror("SIOCSARP");
	return(-1);
  }
#endif

  return(0);
}


/* Process an EtherFile */
static int
arp_file(char *name)
{
  char buff[1024];
  char *sp, *args[32];
  int linenr, argc;
  FILE *fp;

  if ((fp = fopen(name, "r")) == NULL) {
	fprintf(stderr, NLS_CATGETS(catfd, arpSet, arp_cant_open, "arp: cannot open etherfile %s !\n"), name);
	return(-1);
  }

  /* Read the lines in the file. */
  linenr = 0;
  while (fgets(buff, sizeof(buff), fp) != (char *)NULL) {
	linenr++;
	if (opt_v == 1) fprintf(stderr, ">> %s", buff);
	if ((sp = strchr(buff, '\n')) != (char *)NULL) *sp = '\0';
	if (buff[0] == '#' || buff[0] == '\0') continue;

	argc = getargs(buff, args);
	if (argc < 2) {
		fprintf(stderr, NLS_CATGETS(catfd, arpSet, arp_formaterr, 
					    "arp: format error on line %u of etherfile %s !\n"),
			linenr, name);
		continue;
	}

	if (arp_set(args) != 0) {
		fprintf(stderr, NLS_CATGETS(catfd, arpSet, arp_cant_set,
					    "arp: cannot set entry on line %u of etherfile %s !\n"),
			linenr, name);
	}
  }

  (void) fclose(fp);
  return(0);
}


/* Print the contents of an ARP request block. */
static void
arp_disp_2(char *ip,int type,int arp_flags,char *hwa,char *mask,char *dev)
{
  static int title = 0;
  struct hwtype *xhw;
  struct aftype *xap;
  char *sp;
  struct sockaddr sap; 
  char flags[6];
  
  xhw = get_hwntype(type);
  if (xhw == NULL) 
    xhw = get_hwtype("ether");
/*
 * xap = get_afntype(req->arp_pa.sa_family);
 * if (xap == NULL) 
 */
  xap = get_aftype("inet");
    
  if (title++ == 0) {
    printf(NLS_CATGETS(catfd, arpSet, arp_address,
		       "Address\t\t\tHWtype\tHWaddress\t    Flags Mask\t\t  Iface\n"));
  }
  /* Setup the flags. */
  flags[0] = '\0';
  if (arp_flags & ATF_COM) strcat(flags, "C");
  if (arp_flags & ATF_PERM) strcat(flags, "M");
  if (arp_flags & ATF_PUBL) strcat(flags, "P");
/*  if (arp_flags & ATF_RARP) strcat(flags, "R");*/
  if (arp_flags & ATF_USETRAILERS) strcat(flags, "T");

  /* This IS ugly but it works -be */
  if (xap->input(0, ip,&sap) < 0)
  	sp=ip;
  else
  	sp = xap->sprint(&sap, opt_n);
  
  printf("%-23.23s\t%-8.8s", sp, xhw->name);
  printf("%-20.20s%-6.6s%-15.15s %s\n", hwa, flags,mask,dev);
}


/* Display the contents of the ARP cache in the kernel. */
static int
arp_show(char *name)
{
  char host[100];
  struct sockaddr sa;
  char ip[100];
  char hwa[100];
  char mask[100];
  char line[200];
  char dev[100];
  int type,flags;
  FILE *fp;
  int num,entries=0,showed=0;

  host[0]='\0';
  
  if (name != NULL) {
  	/* Resolve the host name. */
  	strcpy(host, name);
  	if (ap->input(0, host, &sa) < 0) {
		ap->herror(host);
		return(-1);
  	}
  	strcpy(host,ap->sprint(&sa, 1));
  }
  
  /* Open the PROCps kernel table. */
  if ((fp = fopen(_PATH_PROCNET_ARP, "r")) == NULL) {
	perror(_PATH_PROCNET_ARP);
	return(-1);
  }

  /* Bypass header -- read until newline */
  if (fgets(line, sizeof(line), fp) != (char *)NULL) {
	strcpy(mask,"-");
	strcpy(dev,"-");
	/* Read the ARP cache entries. */
	for(;fgets(line,sizeof(line),fp);)
	{
		num=sscanf(line,"%s 0x%x 0x%x %s %s %s\n",
		                               ip,&type,&flags,hwa,mask,dev);
		if(num<4)
			break;
		
		entries++;	
		/* if the user specified hw-type differs, skip it */
		if (hw_set && (type != hw->type))
			continue;
			
		/* if the user specified address differs, skip it */
		if (host[0] && strcmp(ip,host))
			continue;
		
		/* if the user specified device differs, skip it */
		if (device[0] && strcmp(dev,device))
			continue;
		showed++;	
		arp_disp_2(ip,type,flags,hwa,mask,dev);
	}
  }
  if (opt_v)
	printf(NLS_CATGETS(catfd, arpSet, arp_sum,
		"Entries: %d\tSkiped: %d\tFound: %d\n"),entries,entries-showed,showed);
  
  if (!showed && (hw_set || host[0] || device[0]))
	printf(NLS_CATGETS(catfd, arpSet, arp_none,
		"arp: in %d entries no match found.\n"),entries);
  (void) fclose(fp);
  return(0);
}

static void
version(void)
{
  fprintf(stderr, "%s\n%s\n%s\n",Release,Version,Features);
  NLS_CATCLOSE(catfd)
  exit(-1);
}

static void
usage(void)
{
  fprintf(stderr, NLS_CATGETS(catfd, arpSet, arp_usage1,
	"Usage: arp [-vn] [-H type] [-i if] -a [hostname]\n"));
  fprintf(stderr, NLS_CATGETS(catfd, arpSet, arp_usage2,
	"       arp [-v] [-i if] -d hostname [pub]\n"));
  fprintf(stderr, NLS_CATGETS(catfd, arpSet, arp_usage3,
	"       arp [-v] [-H type] [-i if] -s  hostname hw_addr [temp]\n"));
  fprintf(stderr, NLS_CATGETS(catfd, arpSet, arp_usage4,
	"       arp [-v] [-H type] [-i if] -s  hostname hw_addr [netmask nm] pub\n"));
  fprintf(stderr, NLS_CATGETS(catfd, arpSet, arp_usage5,
	"       arp [-v] [-H type] [-i if] -Ds hostname if [netmask nm] pub\n"));
  fprintf(stderr, NLS_CATGETS(catfd, arpSet, arp_usage6,
	"       arp [-vnD] [-H type] [-i if] -f filename\n"));
  NLS_CATCLOSE(catfd)
  exit(-1);
}

int
main(int argc, char **argv)
{
  int i, lop, what;
  struct option longopts[]=
  {
	{"verbose",	0,	0,	'v'},
	{"version",	0,	0,	'V'},
	{"display",	0,	0,	'a'},
	{"delete",	0,	0,	'd'},
	{"file",	0,	0,	'f'},
	{"numeric",	0,	0,	'n'},
	{"set",		0,	0,	's'},
	{"protocol",	1,	0,	'A'},
	{"hw-type",	1,	0,	'H'},
	{"device",	0,	0,	'i'},
	{"help",	0,	0,	'h'},
	{"use-device",	0,	0,	'D'},
	{"symbolic",	0,	0,	'N'},
	{NULL,		0,	0,	0}
  };	
             
#if NLS
  setlocale (LC_MESSAGES, "");
  catfd = catopen ("nettools", MCLoadBySet);
#endif

  /* Initialize variables... */
  if ((hw = get_hwtype(DFLT_HW)) == NULL) {
	fprintf(stderr, NLS_CATGETS(catfd, arpSet, arp_hw_not_supp,
				    "%s: hardware type not supported!\n"), DFLT_HW);
	NLS_CATCLOSE(catfd)
	return(-1);
  }
  if ((ap = get_aftype(DFLT_AF)) == NULL) {
	fprintf(stderr, NLS_CATGETS(catfd, arpSet, arp_fam_not_supp,
				    "%s: address family not supported!\n"), DFLT_AF);
	NLS_CATCLOSE(catfd)
	return(-1);
  }
  what = -1;

  /* Fetch the command-line arguments. */
  /* opterr = 0; */
  while ((i = getopt_long(argc, argv, "A:H:adfp:nsi:t:vh?DNV",longopts, &lop)) != EOF) switch(i) {
	case 'a':
		what = 1;
		break;

	case 'd':
		what = 3;
		break;

	case 'f':
		what = 2;
		break;

	case 'n':
		opt_n = FLAG_NUM;
		break;
	case 'D':
		opt_D = 1;
		break;
	case 'N':
		opt_N = FLAG_SYM;
		fprintf(stderr,"arp: -N not yet supported.\n");
		break;
	case 'A':
	case 'p':
		ap = get_aftype(optarg);
		if (ap == NULL) {
			fprintf(stderr, NLS_CATGETS(catfd, arpSet, arp_unkn_addr,
						    "arp: %s: unknown address family.\n"),
				optarg);
			NLS_CATCLOSE(catfd)
			exit(-1);
		}
		break;

	case 's':
		what = 4;
		break;

	case 'H':
	case 't':
		hw = get_hwtype(optarg);
		if (hw == NULL) {
			fprintf(stderr, NLS_CATGETS(catfd, arpSet, arp_unkn_hw,
						    "arp: %s: unknown hardware type.\n"),
				optarg);
			NLS_CATCLOSE(catfd)
			exit(-1);
		}
		hw_set = 1;
		break;
	case 'i':
		strncpy(device,optarg,sizeof(device)-1);
		device[sizeof(device)-1]='\0';
		break;

	case 'v':
		opt_v = 1;
		break;

	case 'V':
		version();

	case '?':
	case 'h':
	default:
		usage();
  }

  if (ap->af != AF_INET) {
	fprintf(stderr, NLS_CATGETS(catfd, arpSet, arp_wrong_af,
				    "arp: %s: kernel only supports 'inet'.\n"),
			ap->name);
	NLS_CATCLOSE(catfd)
	exit(-1);
  }
  if (hw->alen <= 0) {
	fprintf(stderr, NLS_CATGETS(catfd, arpSet, arp_wrong_hw,
				    "arp: %s: hardware type without ARP support.\n"),
			hw->name);
	NLS_CATCLOSE(catfd)
	exit(-1);
  }
  if ((sockfd = socket(AF_INET,SOCK_DGRAM,0)) <0)
  {
  	perror("socket");
	NLS_CATCLOSE(catfd)
  	exit(-1);
  }

  /* Now see what we have to do here... */
  switch(what) {
	case 1:		/* show an ARP entry in the cache */
		what = arp_show(argv[optind]);
		break;

	case 2:		/* process an EtherFile */
		what = arp_file(argv[optind]);
		break;

	case 3:		/* delete an ARP entry from the cache */
		what = arp_del(&argv[optind]);
		break;

	case 4:		/* set an ARP entry in the cache */
		what = arp_set(&argv[optind]);
		break;

	default:
		usage();
  }

  NLS_CATCLOSE(catfd)
  exit(what);
}
