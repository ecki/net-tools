/* sockets.c */

#include <sys/socket.h>
#include <stdio.h>

#include "config.h"
#include "sockets.h"

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

int sockets_open(void)
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
