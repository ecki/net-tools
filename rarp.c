/*
 * rarp		This file contains an implementation of the command
 *		that maintains the kernel's RARP cache.  It is derived
 *              from Fred N. van Kempen's arp command.
 *
 * Usage:       rarp -d hostname                      Delete entry
 *		rarp -s hostname ethernet_address     Add entry
 *              rarp -a                               Print entries
 *
 * Rewritten: Phil Blundell <Philip.Blundell@pobox.com>  1997-08-03
 *
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <net/if_arp.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>

#include "config.h"
#include "net-locale.h"
#include "net-support.h"
#include "version.h"
#include "pathnames.h"

#define NO_RARP_MESSAGE         "This kernel does not support RARP.\n"

static char version_string[] = RELEASE "\nrarp 1.01 (1998-01-02)\n";

static struct hwtype *hardware = NULL;

/* Delete an entry from the RARP cache. */
static int rarp_delete(int fd, struct hostent *hp)
{
  struct arpreq req;
  struct sockaddr_in *si;
  unsigned int found = 0;
  char **addr;

  /* The host can have more than one address, so we loop on them. */
  for (addr = hp->h_addr_list; *addr != NULL; addr++) { 
    memset((char *) &req, 0, sizeof(req));
    si = (struct sockaddr_in *) &req.arp_pa;
    si->sin_family = hp->h_addrtype;
    memcpy((char *) &si->sin_addr, *addr, hp->h_length);
    
    /* Call the kernel. */
    if (ioctl(fd, SIOCDRARP, &req) == 0) {
      found++;
    } else {
      switch (errno) {
      case ENXIO:
	break;
      case ENODEV:
	fputs(NO_RARP_MESSAGE, stderr);
	return 1;
      default:
	perror("SIOCDRARP");
	return 1;
      }
    }
  }

  if (found == 0) 
    printf(NLS_CATGETS(catfd, rarpSet, rarp_noentry, 
		       "no RARP entry for %s.\n"), hp->h_name);
  return 0;
}


/* Set an entry in the RARP cache. */
static int rarp_set(int fd, struct hostent *hp, char *hw_addr)
{
  struct arpreq req;
  struct sockaddr_in *si;
  struct sockaddr sap;

  if (hardware->input(hw_addr, &sap)) {
    fprintf(stderr, "%s: bad hardware address\n", hw_addr);
    return 1;
  }

  /* Clear and fill in the request block. */
  memset((char *) &req, 0, sizeof(req));
  si = (struct sockaddr_in *) &req.arp_pa;
  si->sin_family = hp->h_addrtype;
  memcpy((char *) &si->sin_addr, hp->h_addr_list[0], hp->h_length);
  req.arp_ha.sa_family = hardware->type;
  memcpy(req.arp_ha.sa_data, sap.sa_data, hardware->alen);

  /* Call the kernel. */
  if (ioctl(fd, SIOCSRARP, &req) < 0) {
    if (errno == ENODEV)
      fputs(NO_RARP_MESSAGE, stderr);
    else
      perror("SIOCSRARP");
    return  1;
  }
  return 0;
}

/* Process an EtherFile */
static int rarp_file(int fd, const char *name)
{
  char buff[1024];
  char *host, *addr;
  int linenr;
  FILE *fp;
  struct hostent *hp;

  if ((fp = fopen(name, "r")) == NULL) {
    fprintf(stderr, NLS_CATGETS(catfd, rarpSet, 
	      rarp_cant_open, "rarp: cannot open file %s:%s.\n"), name, strerror(errno));
    return -1;
  }

  /* Read the lines in the file. */
  linenr = 0;
  while (fgets(buff, sizeof(buff), fp)) {
    ++linenr;
    if (buff[0] == '#' || buff[0] == '\0') 
      continue;
    if ((addr = strtok(buff, "\n \t")) == NULL) 
      continue;
    if ((host = strtok(NULL, "\n \t")) == NULL) {
      fprintf(stderr, NLS_CATGETS(catfd, rarpSet, rarp_formaterr, 
				  "rarp: format error at %s:%u\n"),
	      name, linenr);
      continue;
    }
    
    if ((hp = gethostbyname(host)) == NULL) {
      fprintf(stderr, NLS_CATGETS(catfd, rarpSet, rarp_unkn_host, 
				  "rarp: %s: unknown host\n"), host);
    }
    if (rarp_set(fd,hp,addr) != 0) {
      fprintf(stderr, NLS_CATGETS(catfd, rarpSet, rarp_cant_set,
				  "rarp: cannot set entry from %s:%u\n"),
	      name, linenr);
    }
  }
  
  (void) fclose(fp);
  return 0;
}
 
static int display_cache(void)
{
  FILE *fd = fopen(_PATH_PROCNET_RARP, "r");
  char buffer[256];
  if (fd == NULL) {
    if (errno == ENOENT) 
      fputs(NO_RARP_MESSAGE, stderr);
    else
      perror(_PATH_PROCNET_RARP);
    return 1;
  }
  while (feof(fd) == 0) {
    if (fgets(buffer, 255, fd))
      fputs(buffer, stdout);
  }
  fclose(fd);
  return 0;
}

static void usage(void)
{
  fprintf(stderr, NLS_CATGETS(catfd, rarpSet, rarp_usage1,
  "Usage: rarp -a                               list entries in cache.\n"));
  fprintf(stderr, NLS_CATGETS(catfd, rarpSet, rarp_usage2,
  "       rarp -d hostname                      delete entry from cache.\n"));
  fprintf(stderr, NLS_CATGETS(catfd, rarpSet, rarp_usage3,
  "       rarp [-t hwtype] -s hostname hwaddr   add entry to cache.\n"));
  fprintf(stderr, NLS_CATGETS(catfd, rarpSet, rarp_usage4,
  "       rarp -V                               display program version.\n"));
  NLS_CATCLOSE(catfd)
  exit(-1);
}

#define MODE_DISPLAY   1
#define MODE_DELETE    2
#define MODE_SET       3
#define MODE_ETHERS    4

static struct option longopts[] = { 
  { "version", 0, NULL, 'V' },
  { "verbose", 0, NULL, 'v' },
  { "list",    0, NULL, 'a' },
  { "set",     0, NULL, 's' },
  { "delete",  0, NULL, 'd' },
  { "help",    0, NULL, 'h' },
  { NULL,      0, NULL, 0 }
};
  
int main(int argc, char **argv)
{
  int result = 0, mode = 0, c, nargs = 0, verbose = 0;
  char *args[3];
  struct hostent *hp;
  int fd;

#if NLS
  setlocale (LC_MESSAGES, "");
  catfd = catopen ("nettools", MCLoadBySet);
#endif

#if HAVE_HWETHER
  /* Get a default hardware type.  */
  hardware = get_hwtype("ether");
#endif

  do {
    c = getopt_long(argc, argv, "-ht:adsVvf", longopts, NULL);
    switch (c) {
    case EOF:
      break;
    case 'h':
      usage();
    case 'V':
      fprintf(stderr, version_string);
      NLS_CATCLOSE(catfd)
      exit(1);
      break;
    case 'v':
      verbose++;
      break;
    case 'a':
    case 's':
    case 'd':
      if (mode) {
	fprintf(stderr, "%s: illegal option mix.\n", argv[0]);
	usage();
      } else {
	mode = (c == 'a'?MODE_DISPLAY:(c == 'd'?MODE_DELETE:MODE_SET));
      }
      break;
    case 'f':
      mode=MODE_ETHERS;
      break;
    case 't':
      if (optarg) {
	hardware = get_hwtype(optarg);
      } else {
	usage();
      }
      break;
    case 1:
      if (nargs == 2) {
	usage();
	exit(1);
      } else {
	args[nargs++] = optarg;
      }
      break;
    default:
      usage();
    }
  } while (c != EOF);

  if (hardware == NULL) {
    fprintf(stderr, NLS_CATGETS(catfd, rarpSet, rarp_unkn_hw,
				"rarp: %s: unknown hardware type.\n"), optarg);
    NLS_CATCLOSE(catfd)
      exit(1);
  }

  switch (mode) {
  case 0:
    usage();

  case MODE_DISPLAY:
  if (nargs != (mode-1)) {
    usage();
  }
    result = display_cache();
    break;

  case MODE_DELETE:
  case MODE_SET:
  if (nargs != (mode-1)) {
    usage();
  }
    if ((hp = gethostbyname(args[0])) == NULL) {
      fprintf(stderr, NLS_CATGETS(catfd, rarpSet, rarp_unkn_host, 
				  "rarp: %s: unknown host\n"), args[0]);
      NLS_CATCLOSE(catfd)
	exit(1);
    }
    if (fd = socket(PF_INET, SOCK_DGRAM, 0), fd < 0) {
      perror("socket");
      NLS_CATCLOSE(catfd)
	exit (1);
    }
    result = (mode == MODE_DELETE)?rarp_delete(fd, hp):rarp_set(fd, hp, args[1]);
    close(fd);
    break;

  case MODE_ETHERS:
    if (nargs!=0 && nargs!=1) usage();
    if (fd = socket(PF_INET, SOCK_DGRAM, 0), fd < 0) {
      perror("socket");
      NLS_CATCLOSE(catfd)
	exit (1);
    }
    result = rarp_file(fd, nargs ? args[0] : _PATH_ETHERS);
    close(fd);

  }

  NLS_CATCLOSE(catfd)
    exit(result);
}
