/*
 * hostname	This file contains an implementation of the command
 *		that maintains the hostname and the domainname. It
 *		is also used to show the FQDN and the IP-Addresses.
 *
 * Usage:	hostname [-d|-f|-s|-a|-i|-y]
 *		hostname [-h|-V]
 *		hostname {name|-F file}
 *		dnsdmoainname	
 *		nisdomainname {name|-F file}
 *
 * Version:	hostname 1.96 (1996-02-18)
 *
 * Author:	Peter Tobias <tobias@et-inf.fho-emden.de>
 *
 * Changes:
 *      {1.90}	Peter Tobias :		Added -a and -i options.
 * 	{1.91}	Bernd Eckenfels :	-v,-V rewritten, long_opts 
 *					(major rewrite), usage.
 *960120 {1.95}	Bernd Eckenfels :	-y/nisdomainname - support for get/
 *					setdomainname added 
 *960218 {1.96} Bernd Eckenfels :	netinet/in.h added
 *
 *		This program is free software; you can redistribute it
 *		and/or  modify it under  the terms of  the GNU General
 *		Public  License as  published  by  the  Free  Software
 *		Foundation;  either  version 2 of the License, or  (at
 *		your option) any later version.
 */
#include <stdio.h>
#include <unistd.h>
#include <getopt.h>
#include <string.h>
#include <netdb.h>
#include <errno.h>
#include <sys/param.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "config.h"
#include "version.h"
#include "net-locale.h"

char *Release = RELEASE,
     *Version = "hostname 1.96 (1996-02-18)";
     
static char *program_name;
static int  opt_v;

static void sethname(char *);
static void setdname(char *);
static void showhname(char *, int);
static void usage(void);
static void version(void);
static void setfilename(char *,int);

#define SETHOST		1
#define SETDOMAIN	2

static void sethname(char *hname)
{
	if (opt_v)
		fprintf(stderr,NLS_CATGETS(catfd, hostnameSet, hostname_verb_set, "Setting hostname to `%s'\n"),
			hname);
	if(sethostname(hname, strlen(hname))) {
		switch(errno) {
			case EPERM:
				fprintf(stderr,NLS_CATGETS(catfd, hostnameSet, hostname_root,
							   "%s: you must be root to change the host name\n"), program_name);
				break;
			case EINVAL:
				fprintf(stderr,NLS_CATGETS(catfd, hostnameSet, hostname_toolong,
							   "%s: name too long\n"), program_name);
				break;
			default:
		}
		NLS_CATCLOSE(catfd)
		exit(1);
	};
}

static void setdname(char *dname)
{
	if (opt_v)
		fprintf(stderr,NLS_CATGETS(catfd, hostnameSet, hostname_verb_dset, "Setting domainname to `%s'\n"),
			dname);
	if(setdomainname(dname, strlen(dname))) {
		switch(errno) {
			case EPERM:
				fprintf(stderr,NLS_CATGETS(catfd, hostnameSet, hostname_dname_root,
							   "%s: you must be root to change the domain name\n"), program_name);
				break;
			case EINVAL:
				fprintf(stderr,NLS_CATGETS(catfd, hostnameSet, hostname_dname_toolong,
							   "%s: name too long\n"), program_name);
				break;
			default:
		}
		NLS_CATCLOSE(catfd)
		exit(1);
	};
}

static void showhname(char *hname, int c)
{
	struct hostent *hp;
	register char *p,**alias;
	struct in_addr **ip;
	
	if (opt_v)
		fprintf(stderr,NLS_CATGETS(catfd, hostnameSet, hostname_verb_res, "Resolving `%s' ...\n"),hname);
	if (!(hp = gethostbyname(hname))) {
		herror(program_name);
		NLS_CATCLOSE(catfd)
		exit(1);
	}

	if (opt_v) { 
		fprintf(stderr,NLS_CATGETS(catfd, hostnameSet, hostname_verb_fnd, "Result: h_name=`%s'\n"),
			hp->h_name);
			
		alias=hp->h_aliases;
		while(alias[0])
			fprintf(stderr,NLS_CATGETS(catfd, hostnameSet, hostname_verb_ali, "Result: h_aliases=`%s'\n"),
				*alias++);
				
		ip=(struct in_addr **)hp->h_addr_list;
		while(ip[0])
			fprintf(stderr,NLS_CATGETS(catfd, hostnameSet, hostname_verb_ipn, "Result: h_addr_list=`%s'\n"),
				inet_ntoa(**ip++));
	}

	if (!(p = strchr(hp->h_name, '.')) && (c == 'd')) return;

	switch(c) {
		case 'a':
			while (hp->h_aliases[0])
				printf("%s ", *hp->h_aliases++);
			printf("\n");
			break;
		case 'i':
			while (hp->h_addr_list[0])
				printf("%s ", inet_ntoa(*(struct in_addr *) * hp->h_addr_list++));
			printf("\n");
			break;
		case 'd':
			printf("%s\n", ++p);
			break;
		case 'f':
			printf("%s\n", hp->h_name);
			break;
		case 's':
			if (p != NULL) *p = '\0';
			printf("%s\n", hp->h_name);
			break;
		default:
	}
}

static void setfilename(char * name,int what)
{		
  register FILE *fd;
  register char *p;
  char fline[MAXHOSTNAMELEN];

  if ((fd = fopen(name, "r")) != NULL) {
	while (fgets(fline, sizeof(fline), fd) != NULL) {
		if ((p = index(fline, '\n')) != NULL)
			*p = '\0';
			if (opt_v) fprintf(stderr, ">> %s\n", fline);
			if (fline[0] == '#')
				continue;
			if (what == SETHOST) {
				sethname(fline);
			} else {
				setdname(fline);
			}
	}
	(void) fclose(fd);
  } else {
	fprintf(stderr,NLS_CATGETS(catfd, hostnameSet, hostname_cant_open, "%s: can't open `%s'\n"),
			program_name, optarg);
	NLS_CATCLOSE(catfd)
	exit(1);
  }
}

static void version(void)
{
	fprintf(stderr,"%s\n%s\n",Release,Version);
	NLS_CATCLOSE(catfd)
	exit(-1);
}

static void usage(void)
{
	fprintf(stderr,NLS_CATGETS(catfd, hostnameSet, hostname_usage1, 
		"Usage: hostname [-v] {hostname|-F file}      set hostname (from file)\n"));
	fprintf(stderr,NLS_CATGETS(catfd, hostnameSet, hostname_usage2, 
		"       domainname [-v] {nisdomain|-F file}   set NIS domainname (from file)\n"));
	fprintf(stderr,NLS_CATGETS(catfd, hostnameSet, hostname_usage3, 
		"       hostname [-v] [-d|-f|-s|-a|-i|-y]     display formated name\n"));
	fprintf(stderr,NLS_CATGETS(catfd, hostnameSet, hostname_usage4, 
		"       hostname [-v]                         display hostname\n\n"));
	fprintf(stderr,NLS_CATGETS(catfd, hostnameSet, hostname_usage5, 
		"       hostname -V|--version|-h|--help       print info and exit\n\n"));
	fprintf(stderr,NLS_CATGETS(catfd, hostnameSet, hostname_usage6, 
		"    dnsdomainname=hostname -d, {yp,nis,}domainname=hostname -y\n\n"));
	fprintf(stderr,NLS_CATGETS(catfd, hostnameSet, hostname_usage7, 
		"    -s, --short           short host name\n"));
	fprintf(stderr,NLS_CATGETS(catfd, hostnameSet, hostname_usage8, 
		"    -a, --alias           alias names\n"));
	fprintf(stderr,NLS_CATGETS(catfd, hostnameSet, hostname_usage9, 
		"    -i, --ip-address      addresses for the hostname\n"));
	fprintf(stderr,NLS_CATGETS(catfd, hostnameSet, hostname_usage10, 
		"    -f, --fqdn, --long    long host name (FQDN)\n"));
	fprintf(stderr,NLS_CATGETS(catfd, hostnameSet, hostname_usage11, 
		"    -d, --domain          DNS domain name\n"));
	fprintf(stderr,NLS_CATGETS(catfd, hostnameSet, hostname_usage12, 
		"    -y, --yp, --nis       NIS/YP domainname\n"));
	fprintf(stderr,NLS_CATGETS(catfd, hostnameSet, hostname_usage13, 
		"    -F, --file            read hostname or nis domainname from given File\n\n"));
	fprintf(stderr,NLS_CATGETS(catfd, hostnameSet, hostname_usage14, 
		"   This comand can get or set the hostname or the NIS domainname. You can\n"));
	fprintf(stderr,NLS_CATGETS(catfd, hostnameSet, hostname_usage15, 
		"   also get the DNS domain or the FQDN (fully qualified domain name).\n"));
	fprintf(stderr,NLS_CATGETS(catfd, hostnameSet, hostname_usage16, 
		"   Unless you are using bind or NIS for host lookups you can change the\n"));
	fprintf(stderr,NLS_CATGETS(catfd, hostnameSet, hostname_usage17, 
		"   FQDN (Fully Qualified Domain Name) and the DNS domain name (which is\n"));
	fprintf(stderr,NLS_CATGETS(catfd, hostnameSet, hostname_usage18, 
		"   part of the FQDN) in the /etc/hosts file.\n"));
	NLS_CATCLOSE(catfd)
	exit(-1);
}


int main(int argc, char **argv)
{
	int c;
	char type='\0';
	int option_index = 0;
	int what = 0;
	char myname[MAXHOSTNAMELEN+1];
	char *file=NULL;

	static const struct option long_options[] =
	{
		{"domain", no_argument, 0, 'd'},
		{"file", required_argument, 0, 'F'},
		{"fqdn", no_argument, 0, 'f'},
		{"help", no_argument, 0, 'h'},
		{"long", no_argument, 0, 'f'},
		{"short", no_argument, 0, 's'},
		{"version", no_argument, 0, 'V'},
		{"verbose", no_argument, 0, 'v'},
		{"alias", no_argument, 0, 'a'},
		{"ip-address", no_argument, 0, 'i'},
		{"nis", no_argument, 0, 'y'},
		{"yp", no_argument, 0, 'y'},
		{0, 0, 0, 0}
	};

#if NLS
	setlocale (LC_MESSAGES, "");
	catfd = catopen ("nettools", MCLoadBySet);
#endif

	program_name = (rindex(argv[0], '/')) ? rindex(argv[0], '/') + 1 : argv[0];

	if (!strcmp(program_name,"ypdomainname") || 
	    !strcmp(program_name,"domainname")   || 
	    !strcmp(program_name,"nisdomainname"))
		what = 3;
	if (!strcmp(program_name,"dnsdomainname"))
		what = 2;
	
	while((c = getopt_long(argc, argv, "adfF:h?isVvy", long_options, &option_index)) != EOF)
	switch(c)
	{
		case 'd':
			what = 2;
			break;
		case 'a':
		case 'f':
		case 'i':
		case 's':
			what = 1;
			type=c;
			break;
		case 'y':
			what = 3;
			break;
		case 'F':
			file=optarg;						
			break;
		case 'v':
			opt_v++;
			break;
		case 'V':
			version();
		case '?':
		case 'h':
		default:
			usage();

	};


	switch(what) {
		case 2:
			if (file || (optind < argc)) {
				fprintf(stderr,NLS_CATGETS(catfd, hostnameSet, hostname_nodns1,
					   "%s: You can't change the DNS domain name with this command\n"), program_name);
				fprintf(stderr,NLS_CATGETS(catfd, hostnameSet, hostname_nodns2,
					   "\nUnless you are using bind or NIS for host lookups you can change the DNS\n"));
				fprintf(stderr,NLS_CATGETS(catfd, hostnameSet, hostname_nodns3,
					   "domain name (which is part of the FQDN) in the /etc/hosts file.\n"));
				NLS_CATCLOSE(catfd)
				exit(1);
			}
			type='d';			
			/* NOBREAK */
		case 0:
			if (file) {
				setfilename(file,SETHOST);
				break;
			}
			if (optind < argc) {
				sethname(argv[optind]);
				break;
			}
		case 1:
			gethostname(myname, sizeof(myname));
			if (opt_v)
				fprintf(stderr,NLS_CATGETS(catfd, hostnameSet, hostname_verb_get, 
					"gethostname()=`%s'\n"),myname);
			if (!type)
				printf("%s\n",myname);
			else
				showhname(myname,type);
			break;
		case 3:
			if (file) {
				setfilename(file,SETDOMAIN);
				break;
			}
			if (optind < argc) {
				setdname(argv[optind]);
				break;
			}
			getdomainname(myname,sizeof(myname));
			if (opt_v)
				fprintf(stderr,NLS_CATGETS(catfd, hostnameSet, hostname_verb_dget, 
					"getdomainname()=`%s'\n"),myname);
			printf("%s\n",myname);
			break;
	}
	NLS_CATCLOSE(catfd)
	exit(0);
}
