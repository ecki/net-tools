/* Code to manipulate interface information, shared between ifconfig and
   netstat. */

#include "config.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>

#if HAVE_AFIPX
#include "ipx.h"
#endif
#include "net-support.h"
#include "pathnames.h"
#include "version.h"
#include "net-locale.h"

#include "interface.h"
#include "sockets.h"

int procnetdev_vsn = 1;

static void 
if_getstats(char *ifname, struct interface *ife)
{
  FILE *f = fopen(_PATH_PROCNET_DEV, "r");
  char buf[256];
  char *bp;

  if (f == NULL)
    return;

  fgets(buf, 255, f);  /* throw away first line of header */
  fgets(buf, 255, f);

  if (strstr(buf, "compressed")) 
    procnetdev_vsn = 3;
  else
    if (strstr(buf, "bytes"))
      procnetdev_vsn = 2;
  
  while (fgets(buf, 255, f)) {
    bp=buf;
    while(*bp && isspace(*bp))
      bp++;
    if (strncmp(bp, ifname, strlen(ifname)) == 0 && 
	bp[strlen(ifname)] == ':') {
      bp = strchr(bp, ':');
      bp++;
      switch (procnetdev_vsn) {
      case 3:
	sscanf(bp, "%ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld",
	       &ife->stats.rx_bytes,
	       &ife->stats.rx_packets,
	       &ife->stats.rx_errors,
	       &ife->stats.rx_dropped,
	       &ife->stats.rx_fifo_errors,
	       &ife->stats.rx_frame_errors,
	       &ife->stats.rx_compressed,
	       &ife->stats.rx_multicast,
	       
	       &ife->stats.tx_bytes,
	       &ife->stats.tx_packets,
	       &ife->stats.tx_errors,
	       &ife->stats.tx_dropped,
	       &ife->stats.tx_fifo_errors,
	       &ife->stats.collisions,
	       &ife->stats.tx_carrier_errors,
	       &ife->stats.tx_compressed
	       );
	break;
      case 2:
	sscanf(bp, "%ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld",
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
	ife->stats.rx_multicast = 0;
	break;
      case 1:
	sscanf(bp, "%ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld",
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
	ife->stats.rx_multicast = 0;
	break;
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
int
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

  if_getstats(ifname, ife);
  return 0;
}
