/* support for ap->rresolv missing */

#include "config.h"

#if HAVE_AFIPX
#include <asm/types.h>
#include <sys/types.h>
#include <sys/socket.h>
#include "ipx.h"
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <ctype.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include "version.h"
#include "net-support.h"
#include "pathnames.h"
#define  EXTERN
#include "net-locale.h"

/* UGLY */

int IPX_rprint(int options)
{
  /* int ext = options & FLAG_EXT; */
  int numeric = options & FLAG_NUM;
  char buff[1024];
  char net[128], router_net[128];
  char router_node[128];
  int num;
  FILE *fp;
  struct aftype *ap;
  struct sockaddr sa;
  
  printf(NLS_CATGETS(catfd, ipxSet, ipx_table, "Kernel IPX routing table\n")); /* xxx */

  if ((ap = get_afntype(AF_IPX)) == NULL) {
  	EINTERN("lib/ipx_rt.c","AF_IPX missing");
  	return(-1);
  }

  printf(NLS_CATGETS(catfd, ipxSet, ipx_header1,
		"Destination               Router Net                Router Node\n"));

  if ((fp = fopen(_PATH_PROCNET_IPX_ROUTE, "r")) == NULL) {
	perror(_PATH_PROCNET_IPX_ROUTE);
	return(-1);
  }

  fgets(buff, 1023, fp);
  
  while (fgets(buff, 1023, fp))
  {
	num = sscanf(buff, "%s %s %s",net,router_net,router_node);
	if (num < 3) continue;
	
	/* Fetch and resolve the Destination */
	(void)ap->input(5,net,&sa);
	strcpy(net, ap->sprint(&sa, numeric));

	/* Fetch and resolve the Router Net */
	(void)ap->input(5,router_net,&sa);
	strcpy(router_net, ap->sprint(&sa, numeric));

	/* Fetch and resolve the Router Node */
	(void)ap->input(2,router_node,&sa);
	strcpy(router_node, ap->sprint(&sa, numeric));

	printf("%-25s %-25s %-25s\n",net, router_net, router_node);
  }

  (void) fclose(fp);
  return(0);
}

#endif /* HAVE_AFIPX */
