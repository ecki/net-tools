/* sockets.c. Rewriten by Andi Kleen. Subject to the GPL. */

#include <sys/socket.h>
#include <stdio.h>
#include <unistd.h>

#include "config.h"
#include "sockets.h"
#include "intl.h"
#include "util.h"

int skfd = -1; 	/* generic raw socket desc.	*/
#if HAVE_AFIPX
int	 ipx_sock = -1;			/* IPX socket			*/
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
#if HAVE_AFECONET
int ec_sock = -1;			/* Econet socket		*/
#endif


struct fam_sock {
	int *varp;
	int  family;
	char *flag_file; 
};

static struct fam_sock sockets[] = { 
	{ &skfd, AF_INET, NULL }, 
#if HAVE_AFIPX
	{ &ipx_sock, AF_IPX, "/proc/net/ipx" }, 
#endif
#if HAVE_AFAX25
	{ &ax25_sock, AF_AX25, "/proc/net/ax25" }, 
#endif
#if HAVE_AFROSE
	{ &rose_sock, AF_ROSE, "/proc/net/rose" }, 
#endif
#if HAVE_AFINET
	{ &inet_sock, AF_INET, NULL },
#endif
#if HAVE_AFINET6
	{ &inet6_sock, AF_INET6, "/proc/net/if_inet6" },  
#endif
#if HAVE_AFATALK
	{ &ddp_sock, AF_APPLETALK, "/proc/net/appletalk" },
#endif
#if HAVE_AFECONET
	{ &ec_sock, AF_ECONET, NULL }, /* XXX */  
#endif
	{ 0 } 
}; 

int sockets_open(int family)
{
	struct fam_sock *sk;
	int sfd = -1; 
	static int force = -1; 

	if (force < 0) {
		force = 0;   
		if (kernel_version() < KRELEASE(2,1,0))
			force = 1;  
		if (access("/proc",R_OK))
			force = 1;
	}
	for (sk = &sockets[0]; sk->varp; sk++) {
		if (family && family != sk->family)
			continue; 
		if (*(sk->varp) != -1) {
			sfd = *(sk->varp);
			continue;
		}
		/* Check some /proc file first to not stress kmod */ 
		if (!force && sk->flag_file) { 
			if (access(sk->flag_file, R_OK))
				continue; 
		}
		sfd = socket(sk->family, SOCK_DGRAM, 0);
		*(sk->varp) = sfd; 
	} 
	if (sfd < 0)  
		fprintf(stderr, _("No usable address families found.\n"));
	return sfd;
}


