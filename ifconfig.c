/*
 * ifconfig   This file contains an implementation of the command
 *              that either displays or sets the characteristics of
 *              one or more of the system's networking interfaces.
 *
 * Version:     $Id: ifconfig.c,v 1.25 1998/12/06 16:17:44 philip Exp $
 *
 * Author:      Fred N. van Kempen, <waltje@uwalt.nl.mugnet.org>
 *              and others.  Copyright 1993 MicroWalt Corporation
 *
 *              This program is free software; you can redistribute it
 *              and/or  modify it under  the terms of  the GNU General
 *              Public  License as  published  by  the  Free  Software
 *              Foundation;  either  version 2 of the License, or  (at
 *              your option) any later version.
 * {1.34} - 19980630 - Arnaldo Carvalho de Melo <acme@conectiva.com.br>
 *                     - gettext instead of catgets for i18n
 *          10/1998  - Andi Kleen. Use interface list primitives.       
 */

#include "config.h"

#include <features.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <net/if_arp.h>
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>

/* Ugh.  But libc5 doesn't provide POSIX types.  */
#include <asm/types.h>

#ifdef HAVE_HWSLIP
#include <linux/if_slip.h>
#endif

#if HAVE_AFINET6

#ifndef _LINUX_IN6_H
/*
 *    This is in linux/include/net/ipv6.h.
 */

struct in6_ifreq {
    struct in6_addr ifr6_addr;
    __u32 ifr6_prefixlen;
    unsigned int ifr6_ifindex;
};

#endif

#define IPV6_ADDR_ANY		0x0000U

#define IPV6_ADDR_UNICAST      	0x0001U
#define IPV6_ADDR_MULTICAST    	0x0002U
#define IPV6_ADDR_ANYCAST	0x0004U

#define IPV6_ADDR_LOOPBACK	0x0010U
#define IPV6_ADDR_LINKLOCAL	0x0020U
#define IPV6_ADDR_SITELOCAL	0x0040U

#define IPV6_ADDR_COMPATv4	0x0080U

#define IPV6_ADDR_SCOPE_MASK	0x00f0U

#define IPV6_ADDR_MAPPED	0x1000U
#define IPV6_ADDR_RESERVED	0x2000U		/* reserved address space */

#endif				/* HAVE_AFINET6 */

#ifdef IFF_PORTSEL
static const char *if_port_text[][4] =
{
  /* Keep in step with <linux/netdevice.h> */
    {"unknown", NULL, NULL, NULL},
    {"10base2", "bnc", "coax", NULL},
    {"10baseT", "utp", "tpe", NULL},
    {"AUI", "thick", "db15", NULL},
    {"100baseT", NULL, NULL, NULL},
    {"100baseTX", NULL, NULL, NULL},
    {"100baseFX", NULL, NULL, NULL},
    {NULL, NULL, NULL, NULL},
};
#endif

#if HAVE_AFIPX
#if (__GLIBC__ > 2) || (__GLIBC__ == 2 && __GLIBC_MINOR__ >= 1)
#include <netipx/ipx.h>
#else
#include "ipx.h"
#endif
#endif
#include "net-support.h"
#include "pathnames.h"
#include "version.h"
#include "../intl.h"
#include "interface.h"
#include "sockets.h"
#include "util.h"

char *Release = RELEASE, *Version = "ifconfig 1.37 (1998-12-05)";

int opt_a = 0;			/* show all interfaces          */
int opt_i = 0;			/* show the statistics          */
int opt_v = 0;			/* debugging output flag        */

int addr_family = 0;		/* currently selected AF        */


void ife_print(struct interface *ptr)
{
    struct aftype *ap;
    struct hwtype *hw;
    int hf;
    int can_compress = 0;
#if HAVE_AFIPX
    static struct aftype *ipxtype = NULL;
#endif
#if HAVE_AFECONET
    static struct aftype *ectype = NULL;
#endif
#if HAVE_AFATALK
    static struct aftype *ddptype = NULL;
#endif
#if HAVE_AFINET6
    FILE *f;
    char addr6[40], devname[20];
    struct sockaddr_in6 sap;
    int plen, scope, dad_status, if_idx;
    extern struct aftype inet6_aftype;
    char addr6p[8][5];

    if (!strncmp(ptr->name, "sit", 3))
	ptr->addr.sa_family = AF_INET6;		/* fix this up properly one day */
#endif

    ap = get_afntype(ptr->addr.sa_family);
    if (ap == NULL)
	ap = get_afntype(0);

    hf = ptr->type;

    if (strncmp(ptr->name, "lo", 2) == 0)
	hf = 255;

    if (hf == ARPHRD_CSLIP || hf == ARPHRD_CSLIP6)
	can_compress = 1;

    hw = get_hwntype(hf);
    if (hw == NULL)
	hw = get_hwntype(-1);

    printf(_("%-9.9s Link encap:%s  "), ptr->name, hw->title);
    /* Don't print the hardware address for ATM or Ash if it's null. */
    if (hw->sprint != NULL && ((strncmp(ptr->name, "atm", 3) &&
				strncmp(ptr->name, "ash", 3)) ||
			       (ptr->hwaddr[0] || ptr->hwaddr[1] || ptr->hwaddr[2] || ptr->hwaddr[3] ||
				ptr->hwaddr[4] || ptr->hwaddr[5])))
	printf(_("HWaddr %s  "), hw->print(ptr->hwaddr));
#ifdef IFF_PORTSEL
    if (ptr->flags & IFF_PORTSEL)
	printf(_("Media:%s%s"), if_port_text[ptr->map.port][0],
	       (ptr->flags & IFF_AUTOMEDIA) ? _("(auto)") : "");
#endif
    printf("\n");
#if HAVE_AFINET6
    if (ap->af != AF_INET6) {
#endif
	printf(_("          %s addr:%s "), ap->name,
	       ap->sprint(&ptr->addr, 1));
	if (ptr->flags & IFF_POINTOPOINT) {
	    printf(_(" P-t-P:%s "), ap->sprint(&ptr->dstaddr, 1));
	}
	if (ptr->flags & IFF_BROADCAST) {
	    printf(_(" Bcast:%s "), ap->sprint(&ptr->broadaddr, 1));
	}
	printf(_(" Mask:%s\n"), ap->sprint(&ptr->netmask, 1));
#if HAVE_AFINET6
    }
    /* FIXME: should be integrated into interface.c.   */

    if ((f = fopen(_PATH_PROCNET_IFINET6, "r")) != NULL) {
	while (fscanf(f, "%4s%4s%4s%4s%4s%4s%4s%4s %02x %02x %02x %02x %20s\n",
		      addr6p[0], addr6p[1], addr6p[2], addr6p[3],
		      addr6p[4], addr6p[5], addr6p[6], addr6p[7],
		  &if_idx, &plen, &scope, &dad_status, devname) != EOF) {
	    if (!strcmp(devname, ptr->name)) {
		sprintf(addr6, "%s:%s:%s:%s:%s:%s:%s:%s",
			addr6p[0], addr6p[1], addr6p[2], addr6p[3],
			addr6p[4], addr6p[5], addr6p[6], addr6p[7]);
		inet6_aftype.input(1, addr6, (struct sockaddr *) &sap);
		printf(_("          inet6 addr: %s/%d"),
		 inet6_aftype.sprint((struct sockaddr *) &sap, 1), plen);
		printf(_(" Scope:"));
		switch (scope) {
		case 0:
		    printf(_("Global"));
		    break;
		case IPV6_ADDR_LINKLOCAL:
		    printf(_("Link"));
		    break;
		case IPV6_ADDR_SITELOCAL:
		    printf(_("Site"));
		    break;
		case IPV6_ADDR_COMPATv4:
		    printf(_("Compat"));
		    break;
		case IPV6_ADDR_LOOPBACK:
		    printf(_("Host"));
		    break;
		default:
		    printf(_("Unknown"));
		}
		printf("\n");
	    }
	}
	fclose(f);
    }
#endif

#if HAVE_AFIPX
    if (ipxtype == NULL)
	ipxtype = get_afntype(AF_IPX);

    if (ipxtype != NULL) {
	if (ptr->has_ipx_bb)
	    printf(_("          IPX/Ethernet II addr:%s\n"),
		   ipxtype->sprint(&ptr->ipxaddr_bb, 1));
	if (ptr->has_ipx_sn)
	    printf(_("          IPX/Ethernet SNAP addr:%s\n"),
		   ipxtype->sprint(&ptr->ipxaddr_sn, 1));
	if (ptr->has_ipx_e2)
	    printf(_("          IPX/Ethernet 802.2 addr:%s\n"),
		   ipxtype->sprint(&ptr->ipxaddr_e2, 1));
	if (ptr->has_ipx_e3)
	    printf(_("          IPX/Ethernet 802.3 addr:%s\n"),
		   ipxtype->sprint(&ptr->ipxaddr_e3, 1));
    }
#endif

#if HAVE_AFATALK
    if (ddptype == NULL)
	ddptype = get_afntype(AF_APPLETALK);
    if (ddptype != NULL) {
	if (ptr->has_ddp)
	    printf(_("          EtherTalk Phase 2 addr:%s\n"), ddptype->sprint(&ptr->ddpaddr, 1));
    }
#endif

#if HAVE_AFECONET
    if (ectype == NULL)
	ectype = get_afntype(AF_ECONET);
    if (ectype != NULL) {
	if (ptr->has_econet)
	    printf(_("          econet addr:%s\n"), ectype->sprint(&ptr->ecaddr, 1));
    }
#endif

    printf("          ");
    if (ptr->flags == 0)
	printf(_("[NO FLAGS] "));
    if (ptr->flags & IFF_UP)
	printf(_("UP "));
    if (ptr->flags & IFF_BROADCAST)
	printf(_("BROADCAST "));
    if (ptr->flags & IFF_DEBUG)
	printf(_("DEBUG "));
    if (ptr->flags & IFF_LOOPBACK)
	printf(_("LOOPBACK "));
    if (ptr->flags & IFF_POINTOPOINT)
	printf(_("POINTOPOINT "));
    if (ptr->flags & IFF_NOTRAILERS)
	printf(_("NOTRAILERS "));
    if (ptr->flags & IFF_RUNNING)
	printf(_("RUNNING "));
    if (ptr->flags & IFF_NOARP)
	printf(_("NOARP "));
    if (ptr->flags & IFF_PROMISC)
	printf(_("PROMISC "));
    if (ptr->flags & IFF_ALLMULTI)
	printf(_("ALLMULTI "));
    if (ptr->flags & IFF_SLAVE)
	printf(_("SLAVE "));
    if (ptr->flags & IFF_MASTER)
	printf(_("MASTER "));
    if (ptr->flags & IFF_MULTICAST)
	printf(_("MULTICAST "));
#ifdef HAVE_DYNAMIC
    if (ptr->flags & IFF_DYNAMIC)
	printf(_("DYNAMIC "));
#endif

    printf(_(" MTU:%d  Metric:%d"),
	   ptr->mtu, ptr->metric ? ptr->metric : 1);
#ifdef SIOCSKEEPALIVE
    if (ptr->outfill || ptr->keepalive)
	printf(_("  Outfill:%d  Keepalive:%d"),
	       ptr->outfill, ptr->keepalive);
#endif
    printf("\n");

    /* If needed, display the interface statistics. */

    if (ptr->statistics_valid) {
	/* XXX: statistics are currently only printed for the primary address,
	 *      not for the aliases, although strictly speaking they're shared
	 *      by all addresses.
	 */
	printf("          ");

	printf(_("RX packets:%lu errors:%lu dropped:%lu overruns:%lu frame:%lu\n"),
	       ptr->stats.rx_packets, ptr->stats.rx_errors,
	       ptr->stats.rx_dropped, ptr->stats.rx_fifo_errors,
	       ptr->stats.rx_frame_errors);
	if (can_compress)
	    printf(_("             compressed:%lu\n"), ptr->stats.rx_compressed);

	printf("          ");

	printf(_("TX packets:%lu errors:%lu dropped:%lu overruns:%lu carrier:%lu\n"),
	       ptr->stats.tx_packets, ptr->stats.tx_errors,
	       ptr->stats.tx_dropped, ptr->stats.tx_fifo_errors,
	       ptr->stats.tx_carrier_errors);
	printf(_("          collisions:%lu "), ptr->stats.collisions);
	if (can_compress)
	    printf(_("compressed:%lu "), ptr->stats.tx_compressed);
	if (ptr->tx_queue_len != -1)
	    printf(_("txqueuelen:%d "), ptr->tx_queue_len);
	printf("\n");
    }

    if ((ptr->map.irq || ptr->map.mem_start || ptr->map.dma ||
	 ptr->map.base_addr)) {
	printf("          ");
	if (ptr->map.irq)
	    printf(_("Interrupt:%d "), ptr->map.irq);
	if (ptr->map.base_addr >= 0x100)	/* Only print devices using it for 
						   I/O maps */
	    printf(_("Base address:0x%x "), ptr->map.base_addr);
	if (ptr->map.mem_start) {
	    printf(_("Memory:%lx-%lx "), ptr->map.mem_start, ptr->map.mem_end);
	}
	if (ptr->map.dma)
	    printf(_("DMA chan:%x "), ptr->map.dma);
	printf("\n");
    }
    printf("\n");
}

static int if_print(char *ifname)
{
    int res; 

    if (!ifname) {
	res = for_all_interfaces(do_if_print, &opt_a);
    } else {
	struct interface *ife;

	ife = lookup_interface(ifname);
	res = do_if_fetch(ife); 
	if (res >= 0) 
	    ife_print(ife);
    }
    return res; 
}


/* Set a certain interface flag. */
static int set_flag(char *ifname, short flag)
{
    struct ifreq ifr;

    strcpy(ifr.ifr_name, ifname);
    if (ioctl(skfd, SIOCGIFFLAGS, &ifr) < 0) {
	fprintf(stderr, _("%s: unknown interface: %s\n"), 
		ifname,	strerror(errno));
	return (-1);
    }
    strcpy(ifr.ifr_name, ifname);
    ifr.ifr_flags |= flag;
    if (ioctl(skfd, SIOCSIFFLAGS, &ifr) < 0) {
	perror("SIOCSIFFLAGS");
	return -1;
    }
    return (0);
}


/* Clear a certain interface flag. */
static int clr_flag(char *ifname, short flag)
{
    struct ifreq ifr;

    strcpy(ifr.ifr_name, ifname);
    if (ioctl(skfd, SIOCGIFFLAGS, &ifr) < 0) {
	fprintf(stderr, _("%s: unknown interface: %s\n"), 
		ifname, strerror(errno));
	return -1;
    }
    strcpy(ifr.ifr_name, ifname);
    ifr.ifr_flags &= ~flag;
    if (ioctl(skfd, SIOCSIFFLAGS, &ifr) < 0) {
	perror("SIOCSIFFLAGS");
	return -1;
    }
    return (0);
}


static void usage(void)
{
    fprintf(stderr, _("Usage: ifconfig [-a] [-i] [-v] interface\n"));
    fprintf(stderr, _("                [[family] address]\n"));
    /* XXX: it would be useful to have the add/del syntax even without IPv6.
       the 2.1 interface address lists make this natural */
#ifdef HAVE_AFINET6
    fprintf(stderr, _("                [add address[/prefixlen]]\n"));
#ifdef SIOCDIFADDR
    fprintf(stderr, _("                [del address[/prefixlen]]\n"));
#endif
    /* XXX the kernel supports tunneling even without ipv6 */
    fprintf(stderr, _("                [tunnel aa.bb.cc.dd]\n"));
#endif
#if HAVE_AFINET
    fprintf(stderr, _("                [[-]broadcast [aa.bb.cc.dd]]\n"));
    fprintf(stderr, _("                [[-]pointopoint [aa.bb.cc.dd]]\n"));
    fprintf(stderr, _("                [netmask aa.bb.cc.dd]\n"));
    fprintf(stderr, _("                [dstaddr aa.bb.cc.dd]\n"));
#endif
    fprintf(stderr, _("                [hw class address]\n"));
    fprintf(stderr, _("                [metric NN] [mtu NN]\n"));
#ifdef SIOCSKEEPALIVE
    fprintf(stderr, _("                [outfill NN] [keepalive NN]\n"));
#endif
    fprintf(stderr, _("                [[-]trailers] [[-]arp]\n"));
    fprintf(stderr, _("                [[-]allmulti] [[-]promisc]\n"));
    fprintf(stderr, _("                [multicast]\n"));
    fprintf(stderr, _("                [mem_start NN] [io_addr NN] [irq NN]\n"));
    fprintf(stderr, _("                [media type]\n"));
#ifdef HAVE_TXQUEUELEN
    fprintf(stderr, _("                [txqueuelen len]\n"));
#endif
#ifdef HAVE_DYNAMIC
    fprintf(stderr, _("                [[-]dynamic]\n"));
#endif
    fprintf(stderr, _("                [up] [down] ...\n"));
    exit(1);
}

static void version(void)
{
    fprintf(stderr, "%s\n%s\n", Release, Version);
    exit(1);
}

static int set_netmask(int skfd, struct ifreq *ifr, struct sockaddr *sa)
{
    int err = 0;

    memcpy((char *) &ifr->ifr_netmask, (char *) sa,
	   sizeof(struct sockaddr));
    if (ioctl(skfd, SIOCSIFNETMASK, ifr) < 0) {
	fprintf(stderr, "SIOCSIFNETMASK: %s\n",
		strerror(errno));
	err = 1;
    }
    return 0;
}

int main(int argc, char **argv)
{
    struct sockaddr sa;
    char host[128];
    struct aftype *ap;
    struct hwtype *hw;
    struct ifreq ifr;
    int goterr = 0, didnetmask = 0;
    char **spp;
    int fd;
#if HAVE_AFINET6
    extern struct aftype inet6_aftype;
    struct sockaddr_in6 sa6;
    struct in6_ifreq ifr6;
    unsigned long prefix_len;
    char *cp;
#endif

#if I18N
    bindtextdomain("net-tools", "/usr/share/locale");
    textdomain("net-tools");
#endif

    /* Create a channel to the NET kernel. */
    if ((skfd = sockets_open(0)) < 0) {
	perror("socket");
	exit(1);
    }

    /* Find any options. */
    argc--;
    argv++;
    while (argc && *argv[0] == '-') {
	if (!strcmp(*argv, "-a"))
	    opt_a = 1;

	if (!strcmp(*argv, "-v"))
	    opt_v = 1;

	if (!strcmp(*argv, "-V") || !strcmp(*argv, "-version") ||
	    !strcmp(*argv, "--version"))
	    version();

	if (!strcmp(*argv, "-?") || !strcmp(*argv, "-h") ||
	    !strcmp(*argv, "-help") || !strcmp(*argv, "--help"))
	    usage();

	argv++;
	argc--;
    }

    /* Do we have to show the current setup? */
    if (argc == 0) {
	int err = if_print((char *) NULL);
	(void) close(skfd);
	exit(err < 0);
    }
    /* No. Fetch the interface name. */
    spp = argv;
    safe_strncpy(ifr.ifr_name, *spp++, IFNAMSIZ);
    if (*spp == (char *) NULL) {
	int err = if_print(ifr.ifr_name);
	(void) close(skfd);
	exit(err < 0);
    }
    /* The next argument is either an address family name, or an option. */
    if ((ap = get_aftype(*spp)) == NULL)
	ap = get_aftype("inet");
    else {
	/* XXX: should print the current setup if no args left, but only 
	   for this family */
	spp++;
	addr_family = ap->af;
    }

    if (sockets_open(addr_family) < 0) {
	perror("family socket");
	exit(1);
    }
    /* Process the remaining arguments. */
    while (*spp != (char *) NULL) {
	if (!strcmp(*spp, "arp")) {
	    goterr |= clr_flag(ifr.ifr_name, IFF_NOARP);
	    spp++;
	    continue;
	}
	if (!strcmp(*spp, "-arp")) {
	    goterr |= set_flag(ifr.ifr_name, IFF_NOARP);
	    spp++;
	    continue;
	}
#ifdef IFF_PORTSEL
	if (!strcmp(*spp, "media") || !strcmp(*spp, "port")) {
	    if (*++spp == NULL)
		usage();
	    if (!strcasecmp(*spp, "auto")) {
		goterr |= set_flag(ifr.ifr_name, IFF_AUTOMEDIA);
	    } else {
		int i, j, newport;
		char *endp;
		newport = strtol(*spp, &endp, 10);
		if (*endp != 0) {
		    newport = -1;
		    for (i = 0; if_port_text[i][0] && newport == -1; i++) {
			for (j = 0; if_port_text[i][j]; j++) {
			    if (!strcasecmp(*spp, if_port_text[i][j])) {
				newport = i;
				break;
			    }
			}
		    }
		}
		spp++;
		if (newport == -1) {
		    fprintf(stderr, _("Unknown media type.\n"));
		    goterr = 1;
		} else {
		    if (ioctl(skfd, SIOCGIFMAP, &ifr) < 0) {
			goterr = 1;
			continue;
		    }
		    ifr.ifr_map.port = newport;
		    if (ioctl(skfd, SIOCSIFMAP, &ifr) < 0) {
			perror("SIOCSIFMAP");
			goterr = 1;
		    }
		}
	    }
	    continue;
	}
#endif

	if (!strcmp(*spp, "trailers")) {
	    goterr |= clr_flag(ifr.ifr_name, IFF_NOTRAILERS);
	    spp++;
	    continue;
	}
	if (!strcmp(*spp, "-trailers")) {
	    goterr |= set_flag(ifr.ifr_name, IFF_NOTRAILERS);
	    spp++;
	    continue;
	}
	if (!strcmp(*spp, "promisc")) {
	    goterr |= set_flag(ifr.ifr_name, IFF_PROMISC);
	    spp++;
	    continue;
	}
	if (!strcmp(*spp, "-promisc")) {
	    goterr |= clr_flag(ifr.ifr_name, IFF_PROMISC);
	    spp++;
	    continue;
	}
	if (!strcmp(*spp, "multicast")) {
	    goterr |= set_flag(ifr.ifr_name, IFF_MULTICAST);
	    spp++;
	    continue;
	}
	if (!strcmp(*spp, "-multicast")) {
	    goterr |= clr_flag(ifr.ifr_name, IFF_MULTICAST);
	    spp++;
	    continue;
	}
	if (!strcmp(*spp, "allmulti")) {
	    goterr |= set_flag(ifr.ifr_name, IFF_ALLMULTI);
	    spp++;
	    continue;
	}
	if (!strcmp(*spp, "-allmulti")) {
	    goterr |= clr_flag(ifr.ifr_name, IFF_ALLMULTI);
	    spp++;
	    continue;
	}
	if (!strcmp(*spp, "up")) {
	    goterr |= set_flag(ifr.ifr_name, (IFF_UP | IFF_RUNNING));
	    spp++;
	    continue;
	}
	if (!strcmp(*spp, "down")) {
	    goterr |= clr_flag(ifr.ifr_name, IFF_UP);
	    spp++;
	    continue;
	}
#ifdef HAVE_DYNAMIC
	if (!strcmp(*spp, "dynamic")) {
	    goterr |= set_flag(ifr.ifr_name, IFF_DYNAMIC);
	    spp++;
	    continue;
	}
	if (!strcmp(*spp, "-dynamic")) {
	    goterr |= clr_flag(ifr.ifr_name, IFF_DYNAMIC);
	    spp++;
	    continue;
	}
#endif

	if (!strcmp(*spp, "metric")) {
	    if (*++spp == NULL)
		usage();
	    ifr.ifr_metric = atoi(*spp);
	    if (ioctl(skfd, SIOCSIFMETRIC, &ifr) < 0) {
		fprintf(stderr, "SIOCSIFMETRIC: %s\n", strerror(errno));
		goterr = 1;
	    }
	    spp++;
	    continue;
	}
	if (!strcmp(*spp, "mtu")) {
	    if (*++spp == NULL)
		usage();
	    ifr.ifr_mtu = atoi(*spp);
	    if (ioctl(skfd, SIOCSIFMTU, &ifr) < 0) {
		fprintf(stderr, "SIOCSIFMTU: %s\n", strerror(errno));
		goterr = 1;
	    }
	    spp++;
	    continue;
	}
#ifdef SIOCSKEEPALIVE
	if (!strcmp(*spp, "keepalive")) {
	    if (*++spp == NULL)
		usage();
	    ifr.ifr_data = (caddr_t) atoi(*spp);
	    if (ioctl(skfd, SIOCSKEEPALIVE, &ifr) < 0) {
		fprintf(stderr, "SIOCSKEEPALIVE: %s\n", strerror(errno));
		goterr = 1;
	    }
	    spp++;
	    continue;
	}
#endif

#ifdef SIOCSOUTFILL
	if (!strcmp(*spp, "outfill")) {
	    if (*++spp == NULL)
		usage();
	    ifr.ifr_data = (caddr_t) atoi(*spp);
	    if (ioctl(skfd, SIOCSOUTFILL, &ifr) < 0) {
		fprintf(stderr, "SIOCSOUTFILL: %s\n", strerror(errno));
		goterr = 1;
	    }
	    spp++;
	    continue;
	}
#endif

	if (!strcmp(*spp, "-broadcast")) {
	    goterr |= clr_flag(ifr.ifr_name, IFF_BROADCAST);
	    spp++;
	    continue;
	}
	if (!strcmp(*spp, "broadcast")) {
	    if (*++spp != NULL) {
		safe_strncpy(host, *spp, (sizeof host));
		if (ap->input(0, host, &sa) < 0) {
		    ap->herror(host);
		    goterr = 1;
		    spp++;
		    continue;
		}
		memcpy((char *) &ifr.ifr_broadaddr, (char *) &sa,
		       sizeof(struct sockaddr));
		if (ioctl(ap->fd, SIOCSIFBRDADDR, &ifr) < 0) {
		    fprintf(stderr, "SIOCSIFBRDADDR: %s\n",
			    strerror(errno));
		    goterr = 1;
		}
		spp++;
	    }
	    goterr |= set_flag(ifr.ifr_name, IFF_BROADCAST);
	    continue;
	}
	if (!strcmp(*spp, "dstaddr")) {
	    if (*++spp == NULL)
		usage();
	    safe_strncpy(host, *spp, (sizeof host));
	    if (ap->input(0, host, &sa) < 0) {
		ap->herror(host);
		goterr = 1;
		spp++;
		continue;
	    }
	    memcpy((char *) &ifr.ifr_dstaddr, (char *) &sa,
		   sizeof(struct sockaddr));
	    if (ioctl(ap->fd, SIOCSIFDSTADDR, &ifr) < 0) {
		fprintf(stderr, "SIOCSIFDSTADDR: %s\n",
			strerror(errno));
		goterr = 1;
	    }
	    spp++;
	    continue;
	}
	if (!strcmp(*spp, "netmask")) {
	    if (*++spp == NULL || didnetmask)
		usage();
	    safe_strncpy(host, *spp, (sizeof host));
	    if (ap->input(0, host, &sa) < 0) {
		ap->herror(host);
		goterr = 1;
		spp++;
		continue;
	    }
	    didnetmask++;
	    goterr = set_netmask(ap->fd, &ifr, &sa);
	    spp++;
	    continue;
	}
#ifdef HAVE_TXQUEUELEN
	if (!strcmp(*spp, "txqueuelen")) {
	    if (*++spp == NULL)
		usage();
	    ifr.ifr_qlen = strtoul(*spp, NULL, 0);
	    if (ioctl(skfd, SIOCSIFTXQLEN, &ifr) < 0) {
		fprintf(stderr, "SIOCSIFTXQLEN: %s\n", strerror(errno));
		goterr = 1;
	    }
	    spp++;
	    continue;
	}
#endif

	if (!strcmp(*spp, "mem_start")) {
	    if (*++spp == NULL)
		usage();
	    if (ioctl(skfd, SIOCGIFMAP, &ifr) < 0) {
		goterr = 1;
		continue;
	    }
	    ifr.ifr_map.mem_start = strtoul(*spp, NULL, 0);
	    if (ioctl(skfd, SIOCSIFMAP, &ifr) < 0) {
		fprintf(stderr, "SIOCSIFMAP: %s\n", strerror(errno));
		goterr = 1;
	    }
	    spp++;
	    continue;
	}
	if (!strcmp(*spp, "io_addr")) {
	    if (*++spp == NULL)
		usage();
	    if (ioctl(skfd, SIOCGIFMAP, &ifr) < 0) {
		goterr = 1;
		continue;
	    }
	    ifr.ifr_map.base_addr = strtol(*spp, NULL, 0);
	    if (ioctl(skfd, SIOCSIFMAP, &ifr) < 0) {
		fprintf(stderr, "SIOCSIFMAP: %s\n", strerror(errno));
		goterr = 1;
	    }
	    spp++;
	    continue;
	}
	if (!strcmp(*spp, "irq")) {
	    if (*++spp == NULL)
		usage();
	    if (ioctl(skfd, SIOCGIFMAP, &ifr) < 0) {
		goterr = 1;
		continue;
	    }
	    ifr.ifr_map.irq = atoi(*spp);
	    if (ioctl(skfd, SIOCSIFMAP, &ifr) < 0) {
		fprintf(stderr, "SIOCSIFMAP: %s\n", strerror(errno));
		goterr = 1;
	    }
	    spp++;
	    continue;
	}
	if (!strcmp(*spp, "-pointopoint")) {
	    goterr |= clr_flag(ifr.ifr_name, IFF_POINTOPOINT);
	    spp++;
	    continue;
	}
	if (!strcmp(*spp, "pointopoint")) {
	    if (*(spp + 1) != NULL) {
		spp++;
		safe_strncpy(host, *spp, (sizeof host));
		if (ap->input(0, host, &sa)) {
		    ap->herror(host);
		    goterr = 1;
		    spp++;
		    continue;
		}
		memcpy((char *) &ifr.ifr_dstaddr, (char *) &sa,
		       sizeof(struct sockaddr));
		if (ioctl(skfd, SIOCSIFDSTADDR, &ifr) < 0) {
		    fprintf(stderr, "SIOCSIFDSTADDR: %s\n",
			    strerror(errno));
		    goterr = 1;
		}
	    }
	    goterr |= set_flag(ifr.ifr_name, IFF_POINTOPOINT);
	    spp++;
	    continue;
	};

	if (!strcmp(*spp, "hw")) {
	    if (*++spp == NULL)
		usage();
	    if ((hw = get_hwtype(*spp)) == NULL)
		usage();
	    if (*++spp == NULL)
		usage();
	    safe_strncpy(host, *spp, (sizeof host));
	    if (hw->input(host, &sa) < 0) {
		fprintf(stderr, _("%s: invalid %s address.\n"), host, hw->name);
		goterr = 1;
		spp++;
		continue;
	    }
	    memcpy((char *) &ifr.ifr_hwaddr, (char *) &sa,
		   sizeof(struct sockaddr));
	    if (ioctl(skfd, SIOCSIFHWADDR, &ifr) < 0) {
		fprintf(stderr, "SIOCSIFHWADDR: %s\n",
			strerror(errno));
		goterr = 1;
	    }
	    spp++;
	    continue;
	}
#if HAVE_AFINET6
	if (!strcmp(*spp, "add")) {
	    if (*++spp == NULL)
		usage();
	    if ((cp = strchr(*spp, '/'))) {
		prefix_len = atol(cp + 1);
		if ((prefix_len < 0) || (prefix_len > 128))
		    usage();
		*cp = 0;
	    } else {
		prefix_len = 0;
	    }
	    safe_strncpy(host, *spp, (sizeof host));
	    if (inet6_aftype.input(1, host, (struct sockaddr *) &sa6) < 0) {
		inet6_aftype.herror(host);
		goterr = 1;
		spp++;
		continue;
	    }
	    memcpy((char *) &ifr6.ifr6_addr, (char *) &sa6.sin6_addr,
		   sizeof(struct in6_addr));

	    fd = get_socket_for_af(AF_INET6);
	    if (fd < 0) {
		fprintf(stderr, _("No support for INET6 on this system.\n"));
		goterr = 1;
		spp++;
		continue;
	    }
	    if (ioctl(fd, SIOGIFINDEX, &ifr) < 0) {
		perror("SIOGIFINDEX");
		goterr = 1;
		spp++;
		continue;
	    }
	    ifr6.ifr6_ifindex = ifr.ifr_ifindex;
	    ifr6.ifr6_prefixlen = prefix_len;
	    if (ioctl(fd, SIOCSIFADDR, &ifr6) < 0) {
		perror("SIOCSIFADDR");
		goterr = 1;
	    }
	    spp++;
	    continue;
	}
	if (!strcmp(*spp, "del")) {
	    if (*++spp == NULL)
		usage();
	    if ((cp = strchr(*spp, '/'))) {
		prefix_len = atol(cp + 1);
		if ((prefix_len < 0) || (prefix_len > 128))
		    usage();
		*cp = 0;
	    } else {
		prefix_len = 0;
	    }
	    safe_strncpy(host, *spp, (sizeof host));
	    if (inet6_aftype.input(1, host, (struct sockaddr *) &sa6) < 0) {
		inet6_aftype.herror(host);
		goterr = 1;
		spp++;
		continue;
	    }
	    memcpy((char *) &ifr6.ifr6_addr, (char *) &sa6.sin6_addr,
		   sizeof(struct in6_addr));

	    fd = get_socket_for_af(AF_INET6);
	    if (fd < 0) {
		fprintf(stderr, _("No support for INET6 on this system.\n"));
		goterr = 1;
		spp++;
		continue;
	    }
	    if (ioctl(fd, SIOGIFINDEX, &ifr) < 0) {
		perror("SIOGIFINDEX");
		goterr = 1;
		spp++;
		continue;
	    }
	    ifr6.ifr6_ifindex = ifr.ifr_ifindex;
	    ifr6.ifr6_prefixlen = prefix_len;
#ifdef SIOCDIFADDR
	    if (ioctl(fd, SIOCDIFADDR, &ifr6) < 0) {
		fprintf(stderr, "SIOCDIFADDR: %s\n",
			strerror(errno));
		goterr = 1;
	    }
#else
	    fprintf(stderr, _("Address deletion not supported on this system.\n"));
#endif
	    spp++;
	    continue;
	}
	if (!strcmp(*spp, "tunnel")) {
	    if (*++spp == NULL)
		usage();
	    if ((cp = strchr(*spp, '/'))) {
		prefix_len = atol(cp + 1);
		if ((prefix_len < 0) || (prefix_len > 128))
		    usage();
		*cp = 0;
	    } else {
		prefix_len = 0;
	    }
	    safe_strncpy(host, *spp, (sizeof host));
	    if (inet6_aftype.input(1, host, (struct sockaddr *) &sa6) < 0) {
		inet6_aftype.herror(host);
		goterr = 1;
		spp++;
		continue;
	    }
	    memcpy((char *) &ifr6.ifr6_addr, (char *) &sa6.sin6_addr,
		   sizeof(struct in6_addr));

	    fd = get_socket_for_af(AF_INET6);
	    if (fd < 0) {
		fprintf(stderr, _("No support for INET6 on this system.\n"));
		goterr = 1;
		spp++;
		continue;
	    }
	    if (ioctl(fd, SIOGIFINDEX, &ifr) < 0) {
		perror("SIOGIFINDEX");
		goterr = 1;
		spp++;
		continue;
	    }
	    ifr6.ifr6_ifindex = ifr.ifr_ifindex;
	    ifr6.ifr6_prefixlen = prefix_len;

	    if (ioctl(fd, SIOCSIFDSTADDR, &ifr6) < 0) {
		fprintf(stderr, "SIOCSIFDSTADDR: %s\n",
			strerror(errno));
		goterr = 1;
	    }
	    spp++;
	    continue;
	}
#endif

	/* If the next argument is a valid hostname, assume OK. */
	safe_strncpy(host, *spp, (sizeof host));

	/* FIXME: sa is too small for INET6 addresses, inet6 should use that too, 
	   broadcast is unexpected */
	if (ap->getmask) {
	    switch (ap->getmask(host, &sa, NULL)) {
	    case -1:
		usage();
		break;
	    case 1:
		if (didnetmask)
		    usage();

		goterr = set_netmask(skfd, &ifr, &sa);
		didnetmask++;
		break;
	    }
	}
	if (ap->input(0, host, &sa) < 0) {
	    ap->herror(host);
	    usage();
	}
	memcpy((char *) &ifr.ifr_addr, (char *) &sa, sizeof(struct sockaddr));
	{
	    int r = 0;		/* to shut gcc up */
	    switch (ap->af) {
#if HAVE_AFINET
	    case AF_INET:
		fd = get_socket_for_af(AF_INET);
		if (fd < 0) {
		    fprintf(stderr, _("No support for INET on this system.\n"));
		    exit(1);
		}
		r = ioctl(fd, SIOCSIFADDR, &ifr);
		break;
#endif
#if HAVE_AFECONET
	    case AF_ECONET:
		fd = get_socket_for_af(AF_ECONET);
		if (fd < 0) {
		    fprintf(stderr, _("No support for ECONET on this system.\n"));
		    exit(1);
		}
		r = ioctl(fd, SIOCSIFADDR, &ifr);
		break;
#endif
	    default:
		fprintf(stderr,
		_("Don't know how to set addresses for family %d.\n"), ap->af);
		exit(1);
	    }
	    if (r < 0) {
		perror("SIOCSIFADDR");
		goterr = 1;
	    }
	}
	goterr |= set_flag(ifr.ifr_name, (IFF_UP | IFF_RUNNING));
	spp++;
    }

    return (goterr);
}
