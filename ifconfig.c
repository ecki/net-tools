/*
 * ifconfig	This file contains an implementation of the command
 *		that either displays or sets the characteristics of
 *		one or more of the system's networking interfaces.
 *
 * Version:	ifconfig 1.31 (1998-01-25)
 *
 * Author:	Fred N. van Kempen, <waltje@uwalt.nl.mugnet.org>
 *              and others.  Copyright 1993 MicroWalt Corporation
 *
 *		This program is free software; you can redistribute it
 *		and/or  modify it under  the terms of  the GNU General
 *		Public  License as  published  by  the  Free  Software
 *		Foundation;  either  version 2 of the License, or  (at
 *		your option) any later version.
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

#if HAVE_AFINET6

#ifndef _LINUX_IN6_H
/*
 *	This is in linux/include/net/ipv6.h.
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
#define IPV6_ADDR_RESERVED	0x2000U	/* reserved address space */

#endif  /* HAVE_AFINET6 */

/* Check for supported features */

#if defined(SIOCSIFTXQLEN) && defined(ifr_qlen)
#define HAVE_TXQUEUELEN
#endif

/* This is from <linux/netdevice.h>. */

struct user_net_device_stats
{
  unsigned long	rx_packets;	/* total packets received	*/
  unsigned long	tx_packets;	/* total packets transmitted	*/
  unsigned long	rx_bytes;	/* total bytes received 	*/
  unsigned long	tx_bytes;	/* total bytes transmitted	*/
  unsigned long	rx_errors;	/* bad packets received		*/
  unsigned long	tx_errors;	/* packet transmit problems	*/
  unsigned long	rx_dropped;	/* no space in linux buffers	*/
  unsigned long	tx_dropped;	/* no space available in linux	*/
  unsigned long	multicast;	/* multicast packets received	*/
  unsigned long	collisions;

  /* detailed rx_errors: */
  unsigned long	rx_length_errors;
  unsigned long	rx_over_errors;	/* receiver ring buff overflow	*/
  unsigned long	rx_crc_errors;	/* recved pkt with crc error	*/
  unsigned long	rx_frame_errors; /* recv'd frame alignment error */
  unsigned long	rx_fifo_errors;	/* recv'r fifo overrun		*/
  unsigned long	rx_missed_errors; /* receiver missed packet	*/
  /* detailed tx_errors */
  unsigned long	tx_aborted_errors;
  unsigned long	tx_carrier_errors;
  unsigned long	tx_fifo_errors;
  unsigned long	tx_heartbeat_errors;
  unsigned long	tx_window_errors;
};

struct interface {
  char			name[IFNAMSIZ];		/* interface name	 */
  short			type;			/* if type		 */
  short			flags;			/* various flags	 */
  int			metric;			/* routing metric	 */
  int			mtu;			/* MTU value		 */
  int			tx_queue_len;		/* transmit queue length */
  struct ifmap		map;			/* hardware setup	 */
  struct sockaddr	addr;			/* IP address		 */
  struct sockaddr	dstaddr;		/* P-P IP address	 */
  struct sockaddr	broadaddr;		/* IP broadcast address	 */
  struct sockaddr	netmask;		/* IP network mask	 */
  struct sockaddr	ipxaddr_bb;		/* IPX network address   */
  struct sockaddr	ipxaddr_sn;		/* IPX network address   */
  struct sockaddr	ipxaddr_e3;		/* IPX network address   */
  struct sockaddr	ipxaddr_e2;		/* IPX network address   */
  struct sockaddr	ddpaddr;		/* Appletalk DDP address */
  int			has_ip;
  int			has_ipx_bb;
  int			has_ipx_sn;
  int			has_ipx_e3;
  int			has_ipx_e2;
  int			has_ax25;
  int			has_ddp;
  char			hwaddr[32];		/* HW address		 */
  struct user_net_device_stats stats;		/* statistics		 */
};

static const char *if_port_text[][4] = {
  /* Keep in step with <linux/netdevice.h> */
  { "unknown", NULL , NULL, NULL },
  { "10base2", "bnc", "coax", NULL },
  { "10baseT", "utp", "tpe", NULL },
  { "AUI", "thick", "db15", NULL },
  { "100baseT", NULL, NULL, NULL },
  { "100baseTX", NULL, NULL, NULL },
  { "100baseFX", NULL, NULL, NULL },
  { NULL, NULL, NULL, NULL },
};

#if HAVE_AFIPX
#include "ipx.h"
#endif
#include "net-support.h"
#include "pathnames.h"
#include "version.h"
#include "net-locale.h"

char *Release = RELEASE,
     *Version = "ifconfig 1.30 ($Id)";

int opt_a = 0;				/* show all interfaces		*/
int opt_i = 0;				/* show the statistics		*/
int opt_v = 0;				/* debugging output flag	*/

int skfd = -1;				/* generic raw socket desc.	*/
#if HAVE_AFIPX
int ipx_sock = -1;			/* IPX socket			*/
#endif
#if HAVE_AFAX25
int ax25_sock = -1;			/* AX.25 socket			*/
#endif
#if HAVE_AFROSE
int rose_sock = -1;			/* Rose socket			*/
#endif
#if HAVE_AFINET
int inet_sock = -1;			/* INET socket			*/
#endif
#if HAVE_AFINET6
int inet6_sock = -1;			/* INET6 socket			*/
#endif
#if HAVE_AFATALK
int ddp_sock = -1;			/* Appletalk DDP socket		*/
#endif
int addr_family = 0;			/* currently selected AF	*/


static void
ife_print(struct interface *ptr)
{
  struct aftype *ap;
  struct hwtype *hw;
  int hf;
  char *dispname=NLS_CATSAVE (catfd, ifconfigSet, ifconfig_over, "overruns");
  static struct aftype *ipxtype=NULL, *ddptype=NULL;
#if HAVE_AFINET6
  FILE *f;
  char addr6[40], devname[10];
  struct sockaddr_in6 sap;
  int plen, scope, dad_status, if_idx;
  extern struct aftype inet6_aftype;
  char addr6p[8][5];
  
  if (!strncmp(ptr->name, "sit", 3)) 
    ptr->addr.sa_family = AF_INET6;      /* fix this up properly one day */
#endif

  ap = get_afntype(ptr->addr.sa_family);
  if (ap == NULL) ap = get_afntype(0);

  hf=ptr->type;

  if(strncmp(ptr->name,"lo",2)==0)
  	hf=255;
  	
  if(hf==ARPHRD_CSLIP || hf==ARPHRD_CSLIP6)
  {
#if NLS
    /* NLS must free dispname */
    free (dispname);
#endif
    /* Overrun got reused: BAD - fix later */
    dispname=NLS_CATSAVE (catfd, ifconfigSet, ifconfig_compress, "compressed");
  }
  
  hw = get_hwntype(hf);
  if (hw == NULL) hw = get_hwntype(-1);

  printf(NLS_CATGETS(catfd, ifconfigSet, ifconfig_link, 
		     "%-8.8s  Link encap:%s  "), ptr->name, hw->title);
  /* Don't print the hardware address for ATM if it's null. */
  if (hw->sprint != NULL && (strncmp(ptr->name, "atm", 3) || 
   (ptr->hwaddr[0] || ptr->hwaddr[1] || ptr->hwaddr[2] || ptr->hwaddr[3] || 
   ptr->hwaddr[4] || ptr->hwaddr[5]))) 
    printf(NLS_CATGETS(catfd, ifconfigSet, ifconfig_hw, "HWaddr %s  ")
	   , hw->print(ptr->hwaddr));
#ifdef IFF_PORTSEL
  if (ptr->flags & IFF_PORTSEL) 
    printf("Media:%s%s", if_port_text[ptr->map.port][0], 
	   (ptr->flags & IFF_AUTOMEDIA)?"(auto)":"");
#endif
  printf("\n");
#if HAVE_AFINET6  
  if (ap->af != AF_INET6) {
#endif
    printf(NLS_CATGETS(catfd, ifconfigSet, ifconfig_adr,
		       "          %s addr:%s"), ap->name,
	   ap->sprint(&ptr->addr, 1));
    if (ptr->flags & IFF_POINTOPOINT) {
      printf(NLS_CATGETS(catfd, ifconfigSet, ifconfig_pap, "  P-t-P:%s  "),
	     ap->sprint(&ptr->dstaddr, 1));
    } else {
      printf(NLS_CATGETS(catfd, ifconfigSet, ifconfig_bcast, "  Bcast:%s  "),
	     ap->sprint(&ptr->broadaddr, 1));
    }
    printf(NLS_CATGETS(catfd, ifconfigSet, ifconfig_mask, "Mask:%s\n"),
	   ap->sprint(&ptr->netmask, 1));
#if HAVE_AFINET6
  }
  if ((f = fopen(_PATH_PROCNET_IFINET6, "r")) != NULL) {
    while(fscanf(f, "%4s%4s%4s%4s%4s%4s%4s%4s %02x %02x %02x %02x %s\n",
		 addr6p[0], addr6p[1], addr6p[2], addr6p[3],
		 addr6p[4], addr6p[5], addr6p[6], addr6p[7],
		 &if_idx, &plen, &scope, &dad_status, devname) != EOF) {
      if (!strcmp(devname, ptr->name)) {
	sprintf(addr6, "%s:%s:%s:%s:%s:%s:%s:%s",
		addr6p[0], addr6p[1], addr6p[2], addr6p[3],
		addr6p[4], addr6p[5], addr6p[6], addr6p[7]);
	inet6_aftype.input(1, addr6, (struct sockaddr *)&sap);
	printf(NLS_CATGETS(catfd, ifconfigSet, ifconfig_adr6,
			   "          inet6 addr: %s/%d"),
	       inet6_aftype.sprint((struct sockaddr *)&sap, 1), plen);
	printf(NLS_CATGETS(catfd, ifconfigSet, ifconfig_scope,
				   " Scope:"));
	switch (scope) {
	case 0: printf("Global"); break;
	case IPV6_ADDR_LINKLOCAL: printf("Link"); break;
	case IPV6_ADDR_SITELOCAL: printf("Site"); break;
	case IPV6_ADDR_COMPATv4: printf("Compat"); break;
	case IPV6_ADDR_LOOPBACK: printf("Host"); break;
	default: printf("Unknown");
	}
	printf("\n");
      }
    }
    fclose(f);
  }
#endif
  
  if (ipxtype==NULL)
    ipxtype=get_afntype(AF_IPX);

  if (ipxtype!=NULL) {
    if(ptr->has_ipx_bb)
      printf(NLS_CATGETS(catfd, ifconfigSet, ifconfig_etherII,
			 "          IPX/Ethernet II addr:%s\n"),
	     ipxtype->sprint(&ptr->ipxaddr_bb,1));
    if(ptr->has_ipx_sn)
      printf(NLS_CATGETS(catfd, ifconfigSet, ifconfig_SNAP,
			 "          IPX/Ethernet SNAP addr:%s\n"),
	     ipxtype->sprint(&ptr->ipxaddr_sn,1));
    if(ptr->has_ipx_e2)
      printf(NLS_CATGETS(catfd, ifconfigSet, ifconfig_8022,
			 "          IPX/Ethernet 802.2 addr:%s\n"),
	     ipxtype->sprint(&ptr->ipxaddr_e2,1));
    if(ptr->has_ipx_e3)
      printf(NLS_CATGETS(catfd, ifconfigSet, ifconfig_8023,
			 "          IPX/Ethernet 802.3 addr:%s\n"),
	     ipxtype->sprint(&ptr->ipxaddr_e3,1));
  }
  if (ddptype==NULL)
    ddptype=get_afntype(AF_APPLETALK);
  if (ddptype!=NULL) {
    if (ptr->has_ddp)
      printf(NLS_CATGETS(catfd, ifconfigSet, ifconfig_talk,
			 "          EtherTalk Phase 2 addr:%s\n"),
	     ddptype->sprint(&ptr->ddpaddr,1));
  }
  printf("          ");
  if (ptr->flags == 0) printf(NLS_CATGETS(catfd, ifconfigSet, ifconfig_noflags,
					  "[NO FLAGS] "));
  if (ptr->flags & IFF_UP) printf("UP ");
  if (ptr->flags & IFF_BROADCAST) printf("BROADCAST ");
  if (ptr->flags & IFF_DEBUG) printf("DEBUG ");
  if (ptr->flags & IFF_LOOPBACK) printf("LOOPBACK ");
  if (ptr->flags & IFF_POINTOPOINT) printf("POINTOPOINT ");
  if (ptr->flags & IFF_NOTRAILERS) printf("NOTRAILERS ");
  if (ptr->flags & IFF_RUNNING) printf("RUNNING ");
  if (ptr->flags & IFF_NOARP) printf("NOARP ");
  if (ptr->flags & IFF_PROMISC) printf("PROMISC ");
  if (ptr->flags & IFF_ALLMULTI) printf("ALLMULTI ");
  if (ptr->flags & IFF_SLAVE) printf("SLAVE ");
  if (ptr->flags & IFF_MASTER) printf("MASTER ");
  if (ptr->flags & IFF_MULTICAST) printf("MULTICAST ");
  printf(NLS_CATGETS(catfd, ifconfigSet, ifconfig_mtu, " MTU:%d  Metric:%d\n"),
	 ptr->mtu, ptr->metric?ptr->metric:1);
  if (ptr->tx_queue_len != -1)
    printf("          txqueuelen:%d\n", ptr->tx_queue_len);

  /* If needed, display the interface statistics. */
  printf("          ");

  printf(NLS_CATGETS(catfd, ifconfigSet, ifconfig_rx,
	"RX packets:%lu errors:%lu dropped:%lu %s:%lu frame:%lu\n"),
	ptr->stats.rx_packets, ptr->stats.rx_errors,
	ptr->stats.rx_dropped, dispname, ptr->stats.rx_fifo_errors,
        ptr->stats.rx_frame_errors);
	 
  printf("          ");

  printf(NLS_CATGETS(catfd, ifconfigSet, ifconfig_tx,
	"TX packets:%lu errors:%lu dropped:%lu %s:%lu carrier:%lu coll:%lu\n"),
	ptr->stats.tx_packets, ptr->stats.tx_errors,
	ptr->stats.tx_dropped, dispname, ptr->stats.tx_fifo_errors,
	ptr->stats.tx_carrier_errors, ptr->stats.collisions);

  if (hf<255 && (ptr->map.irq || ptr->map.mem_start || ptr->map.dma || 
		ptr->map.base_addr)) {
    printf("          ");
    if (ptr->map.irq)
      printf(NLS_CATGETS(catfd, ifconfigSet, ifconfig_interrupt,
			 "Interrupt:%d "), ptr->map.irq);
    if (ptr->map.base_addr>=0x100)      /* Only print devices using it for 
					  I/O maps */
      printf(NLS_CATGETS(catfd, ifconfigSet, ifconfig_base,
			 "Base address:0x%x "), ptr->map.base_addr);
    if (ptr->map.mem_start) {
      printf(NLS_CATGETS(catfd, ifconfigSet, ifconfig_mem, "Memory:%lx-%lx "),
	     ptr->map.mem_start,ptr->map.mem_end);
    }
    if (ptr->map.dma)
      printf(NLS_CATGETS(catfd, ifconfigSet, ifconfig_dma, "DMA chan:%x "),
	     ptr->map.dma);
    printf("\n");
  }
  
  printf("\n");

#if NLS
  /* NLS must free dispname */
  free (dispname);
#endif
}


static void if_getstats(char *ifname, struct interface *ife)
{
  FILE *f = fopen(_PATH_PROCNET_DEV, "r");
  char buf[256];
  int have_byte_counters = 0;
  char *bp;
  if (f==NULL)
    return;
  fgets(buf, 255, f);  /* throw away first line of header */
  fgets(buf, 255, f);
  if (strstr(buf, "bytes")) have_byte_counters=1;
  while(fgets(buf,255,f)) {
    bp=buf;
    while(*bp&&isspace(*bp))
      bp++;
    if(strncmp(bp,ifname,strlen(ifname))==0 && bp[strlen(ifname)]==':') {
      bp=strchr(bp,':');
      bp++;
      if (have_byte_counters) {
	sscanf(bp,"%ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld",
	       &ife->stats.rx_bytes,
	       &ife->stats.rx_packets,
	       &ife->stats.rx_errors,
	       &ife->stats.rx_dropped,
	       &ife->stats.rx_fifo_errors,
	       &ife->stats.rx_frame_errors,
	       
	       &ife->stats.tx_bytes,
	       &ife->stats.tx_packets,
	       &ife->stats.tx_errors,
	       &ife->stats.tx_dropped,
	       &ife->stats.tx_fifo_errors,
	       &ife->stats.collisions,
	       
	       &ife->stats.tx_carrier_errors
	       );
      } else {
	sscanf(bp,"%ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld",
	       &ife->stats.rx_packets,
	       &ife->stats.rx_errors,
	       &ife->stats.rx_dropped,
	       &ife->stats.rx_fifo_errors,
	       &ife->stats.rx_frame_errors,
	       
	       &ife->stats.tx_packets,
	       &ife->stats.tx_errors,
	       &ife->stats.tx_dropped,
	       &ife->stats.tx_fifo_errors,
	       &ife->stats.collisions,
	       
	       &ife->stats.tx_carrier_errors
	       );
	ife->stats.rx_bytes = 0;
	ife->stats.tx_bytes = 0;
      }
      break;
    }
  }
  fclose(f);
}

/* Support for fetching an IPX address */

#if HAVE_AFIPX
static int ipx_getaddr(int sock, int ft, struct ifreq *ifr)
{
	((struct sockaddr_ipx *)&ifr->ifr_addr)->sipx_type=ft;
	return ioctl(sock, SIOCGIFADDR, ifr);
}
#endif

/* Fetch the interface configuration from the kernel. */
static int
if_fetch(char *ifname, struct interface *ife)
{
  struct ifreq ifr;

  memset((char *) ife, 0, sizeof(struct interface));
  strcpy(ife->name, ifname);

  strcpy(ifr.ifr_name, ifname);
  if (ioctl(skfd, SIOCGIFFLAGS, &ifr) < 0) return(-1);
  ife->flags = ifr.ifr_flags;

  strcpy(ifr.ifr_name, ifname);
  if (ioctl(skfd, SIOCGIFHWADDR, &ifr) < 0) 
    memset(ife->hwaddr, 0, 32);
  else 
    memcpy(ife->hwaddr,ifr.ifr_hwaddr.sa_data,8);

  ife->type=ifr.ifr_hwaddr.sa_family;

  strcpy(ifr.ifr_name, ifname);
  if (ioctl(skfd, SIOCGIFMETRIC, &ifr) < 0)
    ife->metric = 0;
  else 
    ife->metric = ifr.ifr_metric;

  strcpy(ifr.ifr_name, ifname);
  if (ioctl(skfd, SIOCGIFMTU, &ifr) < 0)
    ife->mtu = 0;
  else 
    ife->mtu = ifr.ifr_mtu;

  strcpy(ifr.ifr_name, ifname);
  if (ioctl(skfd, SIOCGIFMAP, &ifr) < 0)
    memset(&ife->map, 0, sizeof(struct ifmap));
  else 
    memcpy(&ife->map,&ifr.ifr_map,sizeof(struct ifmap));

  strcpy(ifr.ifr_name, ifname);
  if (ioctl(skfd, SIOCGIFMAP, &ifr) < 0)
    memset(&ife->map, 0, sizeof(struct ifmap));
  else 
    ife->map = ifr.ifr_map;

#ifdef HAVE_TXQUEUELEN
  strcpy(ifr.ifr_name, ifname);
  if (ioctl(skfd, SIOCGIFTXQLEN, &ifr) < 0)
    ife->tx_queue_len = -1; /* unknown value */
  else 
    ife->tx_queue_len = ifr.ifr_qlen;
#else
  ife->tx_queue_len = -1; /* unknown value */
#endif

#if HAVE_AFINET
  strcpy(ifr.ifr_name, ifname);
  if (inet_sock < 0 || ioctl(inet_sock, SIOCGIFDSTADDR, &ifr) < 0)
    memset(&ife->dstaddr, 0, sizeof(struct sockaddr));
  else 
    ife->dstaddr = ifr.ifr_dstaddr;

  strcpy(ifr.ifr_name, ifname);
  if (inet_sock < 0 || ioctl(inet_sock, SIOCGIFBRDADDR, &ifr) < 0)
    memset(&ife->broadaddr, 0, sizeof(struct sockaddr));
  else 
    ife->broadaddr = ifr.ifr_broadaddr;

  strcpy(ifr.ifr_name, ifname);
  if (inet_sock < 0 || ioctl(inet_sock, SIOCGIFNETMASK, &ifr) < 0)
    memset(&ife->netmask, 0, sizeof(struct sockaddr));
  else 
    ife->netmask = ifr.ifr_netmask;

  strcpy(ifr.ifr_name, ifname);
  if (inet_sock < 0 || ioctl(inet_sock, SIOCGIFADDR, &ifr) < 0) 
    memset(&ife->addr, 0, sizeof(struct sockaddr));
  else 
    ife->addr = ifr.ifr_addr;
#endif
  
#if HAVE_AFATALK
  /* DDP address maybe ? */
  strcpy(ifr.ifr_name, ifname);
  if (ddp_sock >= 0 && ioctl(ddp_sock, SIOCGIFADDR, &ifr) == 0) {
    ife->ddpaddr=ifr.ifr_addr;
    ife->has_ddp=1;
  }
#endif

#if HAVE_AFIPX  
  /* Look for IPX addresses with all framing types */
  strcpy(ifr.ifr_name, ifname);
  if (ipx_sock >= 0) {
    if (!ipx_getaddr(ipx_sock, IPX_FRAME_ETHERII, &ifr)) {
      ife->has_ipx_bb=1;
      ife->ipxaddr_bb=ifr.ifr_addr;
    }
    strcpy(ifr.ifr_name, ifname);
    if (!ipx_getaddr(ipx_sock, IPX_FRAME_SNAP, &ifr)) {
      ife->has_ipx_sn=1;
      ife->ipxaddr_sn=ifr.ifr_addr;
    }
    strcpy(ifr.ifr_name, ifname);
    if(!ipx_getaddr(ipx_sock, IPX_FRAME_8023, &ifr)) {
      ife->has_ipx_e3=1;
      ife->ipxaddr_e3=ifr.ifr_addr;
    }
    strcpy(ifr.ifr_name, ifname);
    if(!ipx_getaddr(ipx_sock, IPX_FRAME_8022, &ifr)) {
      ife->has_ipx_e2=1;
      ife->ipxaddr_e2=ifr.ifr_addr;
    }
  }
#endif

  if_getstats(ifname,ife);
  return(0);
}


static void
if_print(char *ifname)
{
  struct interface ife;

  if (ifname == (char *)NULL) {
    int i;
    struct ifconf ifc;
    struct ifreq *ifr;
    ifc.ifc_buf = NULL;
    ifc.ifc_len = 0;
    if (ioctl(skfd, SIOCGIFCONF, &ifc) < 0) {
      /* Can this ever happen? */
      int n = 2, s;
      ifc.ifc_buf = NULL;
      do {
	n *= 2;
	ifc.ifc_buf = realloc(ifc.ifc_buf, (ifc.ifc_len = s = 
					    n*sizeof(struct ifreq)));
	if (ifc.ifc_buf == NULL) {
	  fprintf(stderr, "Out of memory\n");
	  exit(1);
	}
	
	if (ioctl(skfd, SIOCGIFCONF, &ifc) < 0) {
	  perror("SIOCGIFCONF");
	  return;
	}
      } while (ifc.ifc_len == s);
    } else {
      ifc.ifc_buf = malloc(ifc.ifc_len);
      if (ifc.ifc_buf == NULL) {
	fprintf(stderr, "Out of memory.\n");
	exit(1);
      }
      if (ioctl(skfd, SIOCGIFCONF, &ifc) < 0) {
	perror("SIOCGIFCONF");
	return;
      }
    }
    ifr = ifc.ifc_req;
    for (i = ifc.ifc_len / sizeof(struct ifreq); --i >= 0; ifr++) {
      if (if_fetch(ifr->ifr_name, &ife) < 0) {
	fprintf(stderr, NLS_CATGETS(catfd, ifconfigSet, 
		      ifconfig_unkn, "%s: unknown interface.\n"),
		ifr->ifr_name);
	continue;
      }
      
      if (((ife.flags & IFF_UP) == 0) && !opt_a) continue;
      ife_print(&ife);
    }
    free(ifc.ifc_buf);
  } else {
    if (if_fetch(ifname, &ife) < 0)
      fprintf(stderr, NLS_CATGETS(catfd, ifconfigSet, 
		      ifconfig_unkn, "%s: unknown interface.\n"), ifname);
    else 
      ife_print(&ife);
  }
}


/* Set a certain interface flag. */
static int
set_flag(char *ifname, short flag)
{
  struct ifreq ifr;

  strcpy(ifr.ifr_name, ifname);
  if (ioctl(skfd, SIOCGIFFLAGS, &ifr) < 0) {
    fprintf(stderr, "%s: unknown interface.\n", ifname);
    return(-1);
  }
  strcpy(ifr.ifr_name, ifname);
  ifr.ifr_flags |= flag;
  if (ioctl(skfd, SIOCSIFFLAGS, &ifr) < 0) {
    perror("SIOCSIFFLAGS");
    return -1;
  }
  return(0);
}


/* Clear a certain interface flag. */
static int
clr_flag(char *ifname, short flag)
{
  struct ifreq ifr;

  strcpy(ifr.ifr_name, ifname);
  if (ioctl(skfd, SIOCGIFFLAGS, &ifr) < 0) {
    fprintf(stderr, "%s: unknown interface.\n", ifname);
    return -1;
  }
  strcpy(ifr.ifr_name, ifname);
  ifr.ifr_flags &= ~flag;
  if (ioctl(skfd, SIOCSIFFLAGS, &ifr) < 0) {
    perror("SIOCSIFFLAGS");
    return -1;
  }
  return(0);
}


static void
usage(void)
{
  fprintf(stderr, NLS_CATGETS(catfd, ifconfigSet, ifconfig_usage1,
			      "Usage: ifconfig [-a] [-i] [-v] interface\n"));
  fprintf(stderr, "                [[family] address]\n");
#if HAVE_AFINET6
  fprintf(stderr, "                [add inet6address/prefixlen]\n");
#ifdef SIOCDIFADDR
  fprintf(stderr, "                [del inet6address/prefixlen]\n");
#endif
  fprintf(stderr, "                [tunnel aa.bb.cc.dd]\n");
#endif
#if HAVE_AFINET
  fprintf(stderr, "                [[-]broadcast [aa.bb.cc.dd]]\n");
  fprintf(stderr, "                [[-]pointopoint [aa.bb.cc.dd]]\n");
  fprintf(stderr, "                [netmask aa.bb.cc.dd]\n");
  fprintf(stderr, "                [dstaddr aa.bb.cc.dd]\n");
#endif
  fprintf(stderr, "                [hw class address]\n");
  fprintf(stderr, "                [metric NN] [mtu NN]\n");
  fprintf(stderr, "                [[-]trailers] [[-]arp]\n");
  fprintf(stderr, "                [[-]allmulti] [[-]promisc]\n");
  fprintf(stderr, "                [multicast]\n");
  fprintf(stderr, "                [mem_start NN] [io_addr NN] [irq NN]\n");
  fprintf(stderr, "                [media type]\n");
#ifdef HAVE_TXQUEUELEN
  fprintf(stderr, "                [txqueuelen len]\n");
#endif
  fprintf(stderr, "                [up] [down] ...\n");
  NLS_CATCLOSE(catfd)
  exit(1);
}

static void
version(void)
{
  fprintf(stderr,"%s\n%s\n",Release,Version);
  NLS_CATCLOSE(catfd)
  exit(1);
}

static int sockets_open()
{
#if HAVE_AFINET
  inet_sock = socket(AF_INET, SOCK_DGRAM, 0);
#endif

#if HAVE_AFINET6
  inet6_sock = socket(AF_INET6, SOCK_DGRAM, 0);
#endif

#if HAVE_AFIPX 
  ipx_sock = socket(AF_IPX, SOCK_DGRAM, 0);
#endif

#if HAVE_AFAX25 
  ax25_sock = socket(AF_AX25, SOCK_DGRAM, 0);
#endif

#if HAVE_ROSE
  rose_sock = socket(AF_ROSE, SOCK_DGRAM, 0);
#endif

#if HAVE_AFATALK
  ddp_sock = socket(AF_APPLETALK, SOCK_DGRAM, 0);
#endif
  
  /*
   *	Now pick any (existing) useful socket family for generic queries
   */

#if HAVE_AFINET
  if (inet_sock != -1) return inet_sock;
#endif

#if HAVE_AFINET6
  if (inet6_sock != -1) return inet6_sock;
#endif

#if HAVE_AFIPX 
  if (ipx_sock != -1) return ipx_sock;
#endif

#if HAVE_AFAX25 
  if (ax25_sock != -1) return ax25_sock;
#endif

#if HAVE_AFROSE 
  if (rose_sock != -1) return rose_sock;
#endif

#if HAVE_AFATALK
  if (ddp_sock != -1) return ddp_sock;
#endif

  /* We have no address families.  */
  fprintf(stderr, "No usable address families found.\n");
  return -1;
}
	
int
main(int argc, char **argv)
{
  struct sockaddr sa;
  char host[128];
  struct aftype *ap;
  struct hwtype *hw;
  struct ifreq ifr;
  int goterr = 0;
  char **spp;
#if HAVE_AFINET6
  extern struct aftype inet6_aftype;
  struct sockaddr_in6 sa6;
  struct in6_ifreq ifr6;
  unsigned long prefix_len;
  char *cp;
#endif
  
#if NLS
  setlocale (LC_MESSAGES, "");
  catfd = catopen ("nettools", MCLoadBySet);
#endif

  /* Create a channel to the NET kernel. */
  if ((skfd = sockets_open()) < 0) {
    perror("socket");
    NLS_CATCLOSE(catfd)
    exit(1);
  }

  /* Find any options. */
  argc--; argv++;
  while (argc && *argv[0] == '-') {
    if (!strcmp(*argv, "-a")) opt_a = 1;
    
    if (!strcmp(*argv, "-v")) opt_v = 1;
    
    if (!strcmp(*argv, "-V") || !strcmp(*argv, "-version") || 
	!strcmp(*argv, "--version")) version();
    
    if (!strcmp(*argv, "-?") || !strcmp(*argv, "-h") || 
	!strcmp(*argv, "-help") || !strcmp(*argv, "--help")) usage();
    
    argv++;
    argc--;
  }

  /* Do we have to show the current setup? */
  if (argc == 0) {
    if_print((char *)NULL);
    (void) close(skfd);
    NLS_CATCLOSE(catfd)
    exit(0);
  }

  /* No. Fetch the interface name. */
  spp = argv;
  strncpy(ifr.ifr_name, *spp++, IFNAMSIZ);
  if (*spp == (char *)NULL) {
    if_print(ifr.ifr_name);
    (void) close(skfd);
    NLS_CATCLOSE(catfd)
    exit(0);
  }

  /* The next argument is either an address family name, or an option. */
  if ((ap = get_aftype(*spp)) == NULL) 
    ap = get_aftype("inet");
  else 
    spp++;
  addr_family = ap->af;

  /* Process the remaining arguments. */
  while (*spp != (char *)NULL) {
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

    if (!strcmp(*spp, "media") || !strcmp(*spp, "port")) {
      if (*++spp == NULL) usage();
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
	  fprintf(stderr, "Unknown media type.\n");
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

    if (!strcmp(*spp, "metric")) {
      if (*++spp == NULL) usage();
      ifr.ifr_metric = atoi(*spp);
      if (ioctl(skfd, SIOCSIFMETRIC, &ifr) < 0) {
	fprintf(stderr, "SIOCSIFMETRIC: %s\n", strerror(errno));
	goterr = 1;
      }
      spp++;
      continue;
    }
    
    if (!strcmp(*spp, "mtu")) {
      if (*++spp == NULL) usage();
      ifr.ifr_mtu = atoi(*spp);
      if (ioctl(skfd, SIOCSIFMTU, &ifr) < 0) {
	fprintf(stderr, "SIOCSIFMTU: %s\n", strerror(errno));
	goterr = 1;
      }
      spp++;
      continue;
    }
    
    if (!strcmp(*spp, "-broadcast")) {
      goterr |= clr_flag(ifr.ifr_name, IFF_BROADCAST);
      spp++;
      continue;
    }
    
    if (!strcmp(*spp, "broadcast")) {
      if (*++spp != NULL ) {
	strcpy(host, *spp);
	if (ap->input(0, host, &sa) < 0) {
	  ap->herror(host);
	  goterr = 1;
	  spp++;
	  continue;
	}
	memcpy((char *) &ifr.ifr_broadaddr, (char *) &sa,
	       sizeof(struct sockaddr));
	if (ioctl(skfd, SIOCSIFBRDADDR, &ifr) < 0) {
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
      if (*++spp == NULL) usage();
      strcpy(host, *spp);
      if (ap->input(0, host, &sa) < 0) {
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
      spp++;
      continue;
    }
    
    if (!strcmp(*spp, "netmask")) {
      if (*++spp == NULL) usage();
      strcpy(host, *spp);
      if (ap->input(0, host, &sa) < 0) {
	ap->herror(host);
	goterr = 1;
	spp++;
	continue;
      }
      memcpy((char *) &ifr.ifr_netmask, (char *) &sa,
	     sizeof(struct sockaddr));
      if (ioctl(skfd, SIOCSIFNETMASK, &ifr) < 0) {
	fprintf(stderr, "SIOCSIFNETMASK: %s\n",
		strerror(errno));
	goterr = 1;
      }
      spp++;
      continue;
    }

#ifdef HAVE_TXQUEUELEN
    if (!strcmp(*spp, "txqueuelen")) {
      if (*++spp == NULL) usage();
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
      if (*++spp == NULL) usage();
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
      if (*++spp == NULL) usage();
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
      if (*++spp == NULL) usage();
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
      if (*(spp+1) != NULL) {
	spp++;
	strcpy(host, *spp);
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
      if (*++spp == NULL) usage();
      if ((hw = get_hwtype(*spp)) == NULL) usage();
      if (*++spp == NULL) usage();
      strcpy(host, *spp);
      if (hw->input(host, &sa) < 0) {
	fprintf(stderr, "%s: invalid %s address.\n", host, hw->name);
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
      if (*++spp == NULL) usage();
      if ((cp = strchr(*spp, '/'))) {
	prefix_len = atol(cp+1);
	if ((prefix_len < 0) || (prefix_len > 128)) usage();
	*cp = 0;
      } else {
	prefix_len = 0;
      }
      strcpy(host, *spp);
      if (inet6_aftype.input(1, host, (struct sockaddr *)&sa6) < 0) {
	inet6_aftype.herror(host);
	goterr = 1;
	spp++;
	continue;
      }
      memcpy((char *) &ifr6.ifr6_addr, (char *) &sa6.sin6_addr,
	     sizeof(struct in6_addr));
      
      if (ioctl(inet6_sock, SIOGIFINDEX, &ifr) < 0) {
	perror("SIOGIFINDEX");
	goterr = 1;
	spp++;
	continue;
      }

      ifr6.ifr6_ifindex = ifr.ifr_ifindex;
      ifr6.ifr6_prefixlen = prefix_len;
      if (ioctl(inet6_sock, SIOCSIFADDR, &ifr6) < 0) {
	fprintf(stderr, "SIOCSIFADDR: %s\n",
		strerror(errno));
	goterr = 1;
      }
      spp++;
      continue;
    }

    if (!strcmp(*spp, "del")) {
      if (*++spp == NULL) usage();
      if ((cp = strchr(*spp, '/'))) {
	prefix_len = atol(cp+1);
	if ((prefix_len < 0) || (prefix_len > 128)) usage();
	*cp = 0;
      } else {
	prefix_len = 0;
      }
      strcpy(host, *spp);
      if (inet6_aftype.input(1, host, (struct sockaddr *)&sa6) < 0) {
	inet6_aftype.herror(host);
	goterr = 1;
	spp++;
	continue;
      }
      memcpy((char *) &ifr6.ifr6_addr, (char *) &sa6.sin6_addr,
	     sizeof(struct in6_addr));
      
      if (ioctl(inet6_sock, SIOGIFINDEX, &ifr) < 0) {
	perror("SIOGIFINDEX");
	goterr = 1;
	spp++;
	continue;
      }

      ifr6.ifr6_ifindex = ifr.ifr_ifindex;
      ifr6.ifr6_prefixlen = prefix_len;
#ifdef SIOCDIFADDR
      if (ioctl(inet6_sock, SIOCDIFADDR, &ifr6) < 0) {
	fprintf(stderr, "SIOCDIFADDR: %s\n",
		strerror(errno));
	goterr = 1;
      }
#else
      fprintf(stderr, "Address deletion not supported on this system.\n");
#endif
      spp++;
      continue;
    }
    
    if (!strcmp(*spp, "tunnel")) {
      if (*++spp == NULL) usage();
      if ((cp = strchr(*spp, '/'))) {
	prefix_len = atol(cp+1);
	if ((prefix_len < 0) || (prefix_len > 128)) usage();
	*cp = 0;
      } else {
	prefix_len = 0;
      }
      strcpy(host, *spp);
      if (inet6_aftype.input(1, host, (struct sockaddr *)&sa6) < 0) {
	inet6_aftype.herror(host);
	goterr = 1;
	spp++;
	continue;
      }
      memcpy((char *) &ifr6.ifr6_addr, (char *) &sa6.sin6_addr,
	     sizeof(struct in6_addr));
      
      if (ioctl(inet6_sock, SIOGIFINDEX, &ifr) < 0) {
	perror("SIOGIFINDEX");
	goterr = 1;
	spp++;
	continue;
      }
      
      ifr6.ifr6_ifindex = ifr.ifr_ifindex;
      ifr6.ifr6_prefixlen = prefix_len;
      
      if (ioctl(inet6_sock, SIOCSIFDSTADDR, &ifr6) < 0) {
	fprintf(stderr, "SIOCSIFDSTADDR: %s\n",
		strerror(errno));
	goterr = 1;
      }
      spp++;
      continue;
    }
#endif
    
    /* If the next argument is a valid hostname, assume OK. */
    strcpy(host, *spp);
    if (ap->input(0, host, &sa) < 0) {
      ap->herror(host);
      usage();
    }

    memcpy((char *) &ifr.ifr_addr, (char *) &sa, sizeof(struct sockaddr));
    if (ioctl(skfd, SIOCSIFADDR, &ifr) < 0) {
      fprintf(stderr, "SIOCSIFADDR: %s\n", strerror(errno));
      goterr = 1;
    }
    goterr |= set_flag(ifr.ifr_name, (IFF_UP | IFF_RUNNING));
    spp++;
  }

  /* Close the socket. */
  (void) close(skfd);

  NLS_CATCLOSE(catfd)
  return(goterr);
}
