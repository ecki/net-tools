#include "config.h"

#if HAVE_AFIPX
#include <sys/param.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <arpa/nameser.h>
#include <sys/ioctl.h>
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

extern     struct aftype   ipx_aftype;

/* static int skfd = -1; */

/*
static int usage(void)
{
  fprintf(stderr,"IPX usage\n");

  return(E_USAGE);
}
*/

int IPX_rinput(int action, int ext, char **args)
{
  
  fprintf(stderr,"IPX: this needs to be written\n");
  return(0);
}
#endif	/* HAVE_AFIPX */
