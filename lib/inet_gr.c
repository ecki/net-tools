#include "config.h"

#if HAVE_AFINET
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <arpa/nameser.h>
#include <net/route.h>
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
#define  EXTERN
#include "net-locale.h"

#include "net-features.h"

extern     struct aftype   inet_aftype;


int rprint_fib(int ext, int numeric)
{
  char buff[4096], iface[16], flags[16];
  char gate_addr[128], net_addr[128];
  char mask_addr[128];
  struct sockaddr snet, sgate, smask;
  int num, iflags, metric, refcnt, use, mss, window, irtt;
  FILE *fp=fopen(_PATH_PROCNET_ROUTE, "r");

  if (!fp) {
	ESYSNOT("getroute","INET FIB");
	return 1;
  }

  printf(NLS_CATGETS(catfd, inetSet, inet_table, "Kernel IP routing table\n"));

  if (ext == 1)
	printf(NLS_CATGETS(catfd, inetSet, inet_header1,
		"Destination     Gateway         Genmask         "
		"Flags Metric Ref    Use Iface\n"));
  if (ext == 2)
	printf(NLS_CATGETS(catfd, inetSet, inet_header2,
		"Destination     Gateway         Genmask         "
		"Flags   MSS Window  irtt Iface\n"));
  if (ext >= 3)
	printf(NLS_CATGETS(catfd, inetSet, inet_header3,
		"Destination     Gateway         Genmask         "
		"Flags Metric Ref    Use Iface    "
		"MSS   Window irtt\n"));

  irtt=0;
  window=0;
  mss=0;
  while (fgets(buff, 1023, fp))
  {
	num = sscanf(buff, "%s %s %s %X %d %d %d %s %d %d %d\n",
		iface, net_addr, gate_addr,
		&iflags, &refcnt, &use, &metric, mask_addr,
 		&mss,&window,&irtt);
	if (num < 10) continue;

	/* Fetch and resolve the target address. */
	(void)inet_aftype.input(1, net_addr, &snet);
	strcpy(net_addr, inet_aftype.sprint(&snet, (numeric | 0x8000)));
	net_addr[15] = '\0';
    
	/* Fetch and resolve the gateway address. */
	(void)inet_aftype.input(1, gate_addr, &sgate);
	strcpy(gate_addr, inet_aftype.sprint(&sgate, numeric));
	gate_addr[15] = '\0';

	/* Fetch and resolve the genmask. */
	(void)inet_aftype.input(1, mask_addr, &smask);
	strcpy(mask_addr, inet_aftype.sprint(&smask, 1));
	mask_addr[15] = '\0';

	/* Decode the flags. */
	flags[0] = '\0';
	if (iflags & RTF_UP) strcat(flags, "U");
	if (iflags & RTF_GATEWAY) strcat(flags, "G");
#if HAVE_RTF_REJECT
	if (iflags & RTF_REJECT) strcpy(flags,"!");
#endif
	if (iflags & RTF_HOST) strcat(flags, "H");
	if (iflags & RTF_REINSTATE) strcat(flags, "R");
	if (iflags & RTF_DYNAMIC) strcat(flags, "D");
	if (iflags & RTF_MODIFIED) strcat(flags, "M");
	/* Print the info. */
	if (ext == 1) {
#if HAVE_RTF_REJECT
		if (iflags & RTF_REJECT)
			printf("%-15s -               %-15s %-5s %-6d -  %7d -\n",
				net_addr,  mask_addr, flags, metric, use);
		else
#endif
			printf("%-15s %-15s %-15s %-5s %-6d %-2d %7d %s\n",
			net_addr, gate_addr, mask_addr, flags,
			metric, refcnt, use, iface);
	}
	if (ext == 2) {
#if HAVE_RTF_REJECT
		if (iflags & RTF_REJECT)
			printf("%-15s -               %-15s %-5s     - -          - -\n",
				net_addr, mask_addr, flags);
		else
#endif
			printf("%-15s %-15s %-15s %-5s %5d %-5d %6d %s\n",
				net_addr, gate_addr, mask_addr, flags,
				mss, window, irtt, iface);
	}
	if (ext >= 3) {
#if HAVE_RTF_REJECT
		if (iflags & RTF_REJECT)
			printf("%-15s -               %-15s %-5s %-6d -  %7d -        -     -      -\n",
				net_addr, mask_addr, flags, metric, use);
		else
#endif
			printf("%-15s %-15s %-15s %-5s %-6d %-3d %6d %-6.6s   %-5d %-6d %d\n",
			net_addr, gate_addr, mask_addr, flags,
			metric, refcnt, use, iface, mss, window, irtt);
	}
  }

  (void) fclose(fp);
  return(0);
}

int rprint_cache(int ext, int numeric)
{
  char buff[4096], iface[16], flags[16];
  char gate_addr[128], net_addr[128];
  char mask_addr[128];
  struct sockaddr snet, sgate, smask;
  int num, iflags, metric, refcnt, use, mss, window, irtt, hh, arp;

  FILE *fp=fopen(_PATH_PROCNET_RTCACHE, "r");

  if (!fp) {
	ESYSNOT("getroute","INET CACHE");
	return 1;
  }

  if (ext == 1)
	printf(NLS_CATGETS(catfd, inetSet, inet_header1,
		"Destination     Gateway         Source          "
		"Flags Metric Ref    Use Iface\n"));
  if (ext == 2)
	printf(NLS_CATGETS(catfd, inetSet, inet_header2,
		"Destination     Gateway         Source          "
		"Flags   MSS Window  irtt Iface\n"));
  if (ext >= 3)
	printf(NLS_CATGETS(catfd, inetSet, inet_header3,
		"Destination     Gateway         Source          "
		"Flags Metric Ref    Use Iface    "
		"MSS   Window irtt   HH  Arp\n"));

  irtt=0;
  window=0;
  mss=0;
  hh=0;
  arp=0;
  while (fgets(buff, 1023, fp))
  {
	num = sscanf(buff, "%s %s %s %X %d %d %d %s %d %d %d %d %d\n",
		iface, net_addr, gate_addr,
		&iflags, &refcnt, &use, &metric, mask_addr,
		&mss,&window,&irtt,&hh,&arp);
	if (num < 12) continue;

	/* Fetch and resolve the target address. */
	(void)inet_aftype.input(1, net_addr, &snet);
	strcpy(net_addr, inet_aftype.sprint(&snet, (numeric | 0x8000)));
	net_addr[15] = '\0';

	/* Fetch and resolve the gateway address. */
	(void)inet_aftype.input(1, gate_addr, &sgate);
	strcpy(gate_addr, inet_aftype.sprint(&sgate, numeric));
	gate_addr[15] = '\0';

	/* Fetch and resolve the genmask. */
	(void)inet_aftype.input(1, mask_addr, &smask);
	strcpy(mask_addr, inet_aftype.sprint(&smask, 1));
	mask_addr[15] = '\0';

	/* Decode the flags. */
	flags[0] = '\0';
	if (iflags & RTF_UP) strcat(flags, "U");
	if (iflags & RTF_GATEWAY) strcat(flags, "G");
#if HAVE_RTF_REJECT
	if (iflags & RTF_REJECT) strcpy(flags,"!");
#endif
	if (iflags & RTF_HOST) strcat(flags, "H");
	if (iflags & RTF_REINSTATE) strcat(flags, "R");
	if (iflags & RTF_DYNAMIC) strcat(flags, "D");
	if (iflags & RTF_MODIFIED) strcat(flags, "M");
	/* Print the info. */
	if (ext == 1) {
#if HAVE_RTF_REJECT
		if (iflags & RTF_REJECT)
			printf("%-15s -               %-15s %-5s %-6d -  %7d -\n",
				net_addr,  mask_addr, flags, metric, use);
		else
#endif
			printf("%-15s %-15s %-15s %-5s %-6d %-2d %7d %s\n",
			net_addr, gate_addr, mask_addr, flags,
			metric, refcnt, use, iface);
	}
	if (ext == 2) {
#if HAVE_RTF_REJECT
		if (iflags & RTF_REJECT)
			printf("%-15s -               %-15s %-5s     - -          - -\n",
				net_addr, mask_addr, flags);
		else
#endif
			printf("%-15s %-15s %-15s %-5s %5d %-5d %6d %s\n",
				net_addr, gate_addr, mask_addr, flags,
				mss, window, irtt, iface);
	}
	if (ext >= 3) {
#if HAVE_RTF_REJECT
		if (iflags & RTF_REJECT)
			printf("%-15s -               %-15s %-5s %-6d -  %7d -        -     -      -\n",
				net_addr, mask_addr, flags, metric, use);
		else
#endif
			printf("%-15s %-15s %-15s %-5s %-6d %-3d %6d %-6.6s   %-5d %-6d %-6d %-3d %d\n",
			net_addr, gate_addr, mask_addr, flags,
			metric, refcnt, use, iface, mss, window, irtt, 	hh, arp);
	}
  }

  (void) fclose(fp);
  return(0);
}

int INET_rprint(int options)
{
  int ext = options & FLAG_EXT;
  int numeric = options & (FLAG_NUM|FLAG_SYM);
  int rc = E_INTERN;
  
  if (options & FLAG_FIB)
	if ((rc = rprint_fib(ext,numeric)))
  		return(rc);
  if (options & FLAG_CACHE)
  	rc = rprint_cache(ext,numeric);
  
  return(rc);
}

#endif	/* HAVE_AFINET */
