/* Code to manipulate interface information, shared between ifconfig and
   netstat. 

   10/1998 partly rewriten by Andi Kleen to support an interface list.   
   I don't claim that the list operations are efficient @).  

   $Id: interface.c,v 1.8 2000/02/20 17:50:08 philip Exp $
 */

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
#if (__GLIBC__ > 2) || (__GLIBC__ == 2 && __GLIBC_MINOR__ >= 1)
#include <netipx/ipx.h>
#else
#include "ipx.h"
#endif
#endif

#if HAVE_AFECONET
#include <neteconet/ec.h>
#endif

#ifdef HAVE_HWSLIP
#include <linux/if_slip.h>
#include <net/if_arp.h>
#endif

#include "net-support.h"
#include "pathnames.h"
#include "version.h"
#include "proc.h"

#include "interface.h"
#include "sockets.h"
#include "util.h"
#include "intl.h"

int procnetdev_vsn = 1;

static struct interface *int_list;

void add_interface(struct interface *n)
{
    struct interface *ife, **pp;

    pp = &int_list;
    for (ife = int_list; ife; pp = &ife->next, ife = ife->next) {
	if (nstrcmp(ife->name, n->name) > 0)
	    break;
    }
    n->next = (*pp);
    (*pp) = n;
}

struct interface *lookup_interface(char *name, int readlist)
{
    struct interface *ife = NULL;

    if (int_list || (readlist && if_readlist() >= 0)) {
		for (ife = int_list; ife; ife = ife->next) {
	    	if (!strcmp(ife->name, name))
			break;
		}
    }

    if (!ife) { 
	new(ife);
	safe_strncpy(ife->name, name, IFNAMSIZ);  
	add_interface(ife);
    }

    return ife;
}

int for_all_interfaces(int (*doit) (struct interface *, void *), void *cookie)
{
    struct interface *ife;

    if (!int_list && (if_readlist() < 0))
	return -1;
    for (ife = int_list; ife; ife = ife->next) {
	int err = doit(ife, cookie);
	if (err)
	    return err;
    }
    return 0;
}

int free_interface_list(void)
{
    struct interface *ife;
    while ((ife = int_list) != NULL) {
	int_list = ife->next;
	free(ife);
    }
    return 0;
}

static int if_readconf(void)
{
    int numreqs = 30;
    struct ifconf ifc;
    struct ifreq *ifr;
    int n, err = -1;
    int skfd;

    /* SIOCGIFCONF currently seems to only work properly on AF_INET sockets
       (as of 2.1.128) */ 
    skfd = get_socket_for_af(AF_INET);
    if (skfd < 0) {
	fprintf(stderr, _("warning: no inet socket available: %s\n"),
		strerror(errno));
	/* Try to soldier on with whatever socket we can get hold of.  */
	skfd = sockets_open(0);
	if (skfd < 0)
	    return -1;
    }

    ifc.ifc_buf = NULL;
    for (;;) {
	ifc.ifc_len = sizeof(struct ifreq) * numreqs;
	ifc.ifc_buf = xrealloc(ifc.ifc_buf, ifc.ifc_len);

	if (ioctl(skfd, SIOCGIFCONF, &ifc) < 0) {
	    perror("SIOCGIFCONF");
	    goto out;
	}
	if (ifc.ifc_len == sizeof(struct ifreq) * numreqs) {
	    /* assume it overflowed and try again */
	    numreqs += 10;
	    continue;
	}
	break;
    }

    ifr = ifc.ifc_req;
    for (n = 0; n < ifc.ifc_len; n += sizeof(struct ifreq)) {
	lookup_interface(ifr->ifr_name,0);
	ifr++;
    }
    err = 0;

out:
    free(ifc.ifc_buf);
    return err;
}

static char *get_name(char *name, char *p)
{
    while (isspace(*p))
	p++;
    while (*p) {
	if (isspace(*p))
	    break;
	if (*p == ':') {	/* could be an alias */
	    char *dot = p, *dotname = name;
	    *name++ = *p++;
	    while (isdigit(*p))
		*name++ = *p++;
	    if (*p != ':') {	/* it wasn't, backup */
		p = dot;
		name = dotname;
	    }
	    if (*p == '\0')
		return NULL;
	    p++;
	    break;
	}
	*name++ = *p++;
    }
    *name++ = '\0';
    return p;
}

static int procnetdev_version(char *buf)
{
    if (strstr(buf, "compressed"))
	return 3;
    if (strstr(buf, "bytes"))
	return 2;
    return 1;
}

static int get_dev_fields(char *bp, struct interface *ife)
{
    switch (procnetdev_vsn) {
    case 3:
	sscanf(bp,
	"%ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld",
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
	       &ife->stats.tx_compressed);
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
	       &ife->stats.tx_carrier_errors);
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
	       &ife->stats.tx_carrier_errors);
	ife->stats.rx_bytes = 0;
	ife->stats.tx_bytes = 0;
	ife->stats.rx_multicast = 0;
	break;
    }
    return 0;
}

int if_readlist(void)
{
    FILE *fh;
    char buf[512];
    struct interface *ife;
    int err;

    fh = fopen(_PATH_PROCNET_DEV, "r");
    if (!fh) {
		fprintf(stderr, _("Warning: cannot open %s (%s). Limited output.\n"),
			_PATH_PROCNET_DEV, strerror(errno)); 
		return if_readconf();
	}	
    fgets(buf, sizeof buf, fh);	/* eat line */
    fgets(buf, sizeof buf, fh);

#if 0				/* pretty, but can't cope with missing fields */
    fmt = proc_gen_fmt(_PATH_PROCNET_DEV, 1, fh,
		       "face", "",	/* parsed separately */
		       "bytes", "%lu",
		       "packets", "%lu",
		       "errs", "%lu",
		       "drop", "%lu",
		       "fifo", "%lu",
		       "frame", "%lu",
		       "compressed", "%lu",
		       "multicast", "%lu",
		       "bytes", "%lu",
		       "packets", "%lu",
		       "errs", "%lu",
		       "drop", "%lu",
		       "fifo", "%lu",
		       "colls", "%lu",
		       "carrier", "%lu",
		       "compressed", "%lu",
		       NULL);
    if (!fmt)
	return -1;
#else
    procnetdev_vsn = procnetdev_version(buf);
#endif

    err = 0;
    while (fgets(buf, sizeof buf, fh)) {
	char *s;

	new(ife);

	s = get_name(ife->name, buf);
	get_dev_fields(s, ife);
	ife->statistics_valid = 1;

	add_interface(ife);
    }
    if (ferror(fh)) {
	perror(_PATH_PROCNET_DEV);
	err = -1;
    }
    if (!err)
	err = if_readconf();

#if 0
    free(fmt);
#endif
    fclose(fh);
    return err;
}

/* Support for fetching an IPX address */

#if HAVE_AFIPX
static int ipx_getaddr(int sock, int ft, struct ifreq *ifr)
{
    ((struct sockaddr_ipx *) &ifr->ifr_addr)->sipx_type = ft;
    return ioctl(sock, SIOCGIFADDR, ifr);
}
#endif

/* Fetch the interface configuration from the kernel. */
int if_fetch(struct interface *ife)
{
    struct ifreq ifr;
    int fd;
    char *ifname = ife->name; 

    strcpy(ifr.ifr_name, ifname);
    if (ioctl(skfd, SIOCGIFFLAGS, &ifr) < 0)
	return (-1);
    ife->flags = ifr.ifr_flags;

    strcpy(ifr.ifr_name, ifname);
    if (ioctl(skfd, SIOCGIFHWADDR, &ifr) < 0)
	memset(ife->hwaddr, 0, 32);
    else
	memcpy(ife->hwaddr, ifr.ifr_hwaddr.sa_data, 8);

    ife->type = ifr.ifr_hwaddr.sa_family;

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

#ifdef HAVE_HWSLIP
    if (ife->type == ARPHRD_SLIP || ife->type == ARPHRD_CSLIP ||
	ife->type == ARPHRD_SLIP6 || ife->type == ARPHRD_CSLIP6 ||
	ife->type == ARPHRD_ADAPT) {
#ifdef SIOCGOUTFILL
	strcpy(ifr.ifr_name, ifname);
	if (ioctl(skfd, SIOCGOUTFILL, &ifr) < 0)
	    ife->outfill = 0;
	else
	    ife->outfill = (unsigned int) ifr.ifr_data;
#endif
#ifdef SIOCGKEEPALIVE
	strcpy(ifr.ifr_name, ifname);
	if (ioctl(skfd, SIOCGKEEPALIVE, &ifr) < 0)
	    ife->keepalive = 0;
	else
	    ife->keepalive = (unsigned int) ifr.ifr_data;
#endif
    }
#endif

    strcpy(ifr.ifr_name, ifname);
    if (ioctl(skfd, SIOCGIFMAP, &ifr) < 0)
	memset(&ife->map, 0, sizeof(struct ifmap));
    else
	memcpy(&ife->map, &ifr.ifr_map, sizeof(struct ifmap));

    strcpy(ifr.ifr_name, ifname);
    if (ioctl(skfd, SIOCGIFMAP, &ifr) < 0)
	memset(&ife->map, 0, sizeof(struct ifmap));
    else
	ife->map = ifr.ifr_map;

#ifdef HAVE_TXQUEUELEN
    strcpy(ifr.ifr_name, ifname);
    if (ioctl(skfd, SIOCGIFTXQLEN, &ifr) < 0)
	ife->tx_queue_len = -1;	/* unknown value */
    else
	ife->tx_queue_len = ifr.ifr_qlen;
#else
    ife->tx_queue_len = -1;	/* unknown value */
#endif

#if HAVE_AFINET
    /* IPv4 address? */
    fd = get_socket_for_af(AF_INET);
    if (fd >= 0) {
	strcpy(ifr.ifr_name, ifname);
	ifr.ifr_addr.sa_family = AF_INET;
	if (ioctl(fd, SIOCGIFADDR, &ifr) == 0) {
	    ife->has_ip = 1;
	    ife->addr = ifr.ifr_addr;
	    strcpy(ifr.ifr_name, ifname);
	    if (ioctl(fd, SIOCGIFDSTADDR, &ifr) < 0)
	        memset(&ife->dstaddr, 0, sizeof(struct sockaddr));
	    else
	        ife->dstaddr = ifr.ifr_dstaddr;

	    strcpy(ifr.ifr_name, ifname);
	    if (ioctl(fd, SIOCGIFBRDADDR, &ifr) < 0)
	        memset(&ife->broadaddr, 0, sizeof(struct sockaddr));
	    else
		ife->broadaddr = ifr.ifr_broadaddr;

	    strcpy(ifr.ifr_name, ifname);
	    if (ioctl(fd, SIOCGIFNETMASK, &ifr) < 0)
		memset(&ife->netmask, 0, sizeof(struct sockaddr));
	    else
		ife->netmask = ifr.ifr_netmask;
	} else
	    memset(&ife->addr, 0, sizeof(struct sockaddr));
    }
#endif

#if HAVE_AFATALK
    /* DDP address maybe ? */
    fd = get_socket_for_af(AF_APPLETALK);
    if (fd >= 0) {
	strcpy(ifr.ifr_name, ifname);
	if (ioctl(fd, SIOCGIFADDR, &ifr) == 0) {
	    ife->ddpaddr = ifr.ifr_addr;
	    ife->has_ddp = 1;
	}
    }
#endif

#if HAVE_AFIPX
    /* Look for IPX addresses with all framing types */
    fd = get_socket_for_af(AF_IPX);
    if (fd >= 0) {
	strcpy(ifr.ifr_name, ifname);
	if (!ipx_getaddr(fd, IPX_FRAME_ETHERII, &ifr)) {
	    ife->has_ipx_bb = 1;
	    ife->ipxaddr_bb = ifr.ifr_addr;
	}
	strcpy(ifr.ifr_name, ifname);
	if (!ipx_getaddr(fd, IPX_FRAME_SNAP, &ifr)) {
	    ife->has_ipx_sn = 1;
	    ife->ipxaddr_sn = ifr.ifr_addr;
	}
	strcpy(ifr.ifr_name, ifname);
	if (!ipx_getaddr(fd, IPX_FRAME_8023, &ifr)) {
	    ife->has_ipx_e3 = 1;
	    ife->ipxaddr_e3 = ifr.ifr_addr;
	}
	strcpy(ifr.ifr_name, ifname);
	if (!ipx_getaddr(fd, IPX_FRAME_8022, &ifr)) {
	    ife->has_ipx_e2 = 1;
	    ife->ipxaddr_e2 = ifr.ifr_addr;
	}
    }
#endif

#if HAVE_AFECONET
    /* Econet address maybe? */
    fd = get_socket_for_af(AF_ECONET);
    if (fd >= 0) {
	strcpy(ifr.ifr_name, ifname);
	if (ioctl(fd, SIOCGIFADDR, &ifr) == 0) {
	    ife->ecaddr = ifr.ifr_addr;
	    ife->has_econet = 1;
	}
    }
#endif

    return 0;
}

int do_if_fetch(struct interface *ife)
{ 
    if (if_fetch(ife) < 0) {
	char *errmsg; 
	if (errno == ENODEV) { 
	    /* Give better error message for this case. */ 
	    errmsg = _("Device not found"); 
	} else { 
	    errmsg = strerror(errno); 
	}
  	fprintf(stderr, _("%s: error fetching interface information: %s\n"),
		ife->name, errmsg);
	return -1;
    }
    return 0; 
}

int do_if_print(struct interface *ife, void *cookie)
{
    int *opt_a = (int *) cookie;
    int res; 

    res = do_if_fetch(ife); 
    if (res >= 0) {   
	if ((ife->flags & IFF_UP) || *opt_a)
	    ife_print(ife);
    }
    return res;
}
