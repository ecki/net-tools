/*
 * netstat    This file contains an implementation of the command
 *              that helps in debugging the networking modules.
 *
 * NET-TOOLS    A collection of programs that form the base set of the
 *              NET-3 Networking Distribution for the LINUX operating
 *              system.
 *
 * Version:     $Id: netstat.c,v 1.73 2011-04-20 01:35:22 ecki Exp $
 *
 * Authors:     Fred Baumgarten, <dc6iq@insu1.etec.uni-karlsruhe.de>
 *              Fred N. van Kempen, <waltje@uwalt.nl.mugnet.org>
 *              Phil Packer, <pep@wicked.demon.co.uk>
 *              Johannes Stille, <johannes@titan.os.open.de>
 *              Bernd Eckenfels, <net-tools@lina.inka.de>
 *              Phil Blundell <philb@gnu.org>
 *              Tuan Hoang <tqhoang@bigfoot.com>
 *
 * Tuned for NET3 by:
 *              Alan Cox, <A.Cox@swansea.ac.uk>
 *              Copyright (c) 1993  Fred Baumgarten
 *
 * Modified:
 *
 *960116 {1.01} Bernd Eckenfels:        verbose, cleanups
 *960204 {1.10} Bernd Eckenfels:        aftrans, usage, new route_info, 
 *                                      DLFT_AF
 *960204 {1.11} Bernd Eckenfels:        netlink support
 *960204 {1.12} Bernd Eckenfels:        route_init()
 *960215 {1.13} Bernd Eckenfels:        netlink_print honors HAVE_
 *960217 {1.14} Bernd Eckenfels:        masq_info from Jos Vos and 
 *                                      ax25_info from Jonathan Naylor.
 *960218 {1.15} Bernd Eckenfels:        ipx_info rewritten, -e for tcp/ipx
 *960220 {1.16} Bernd Eckenfels:        minor output reformats, -a for -x
 *960221 {1.17} Bernd Eckenfels:        route_init->getroute_init
 *960426 {1.18} Bernd Eckenfels:        new RTACTION, SYM/NUM, FIB/CACHE
 *960517 {1.19} Bernd Eckenfels:        usage() spelling fix and --unix inode, 
 *                                      ':' is part of sock_addr for --inet
 *960822 {x.xx} Frank Strauss:          INET6 support
 *
 *970406 {1.33} Philip Copeland         Added snmp reporting support module -s
 *                                      code provided by Andi Kleen
 *                                      (relly needs to be kernel hooked but 
 *                                      this will do in the meantime)
 *                                      minor header file misplacement tidy up.
 *980815 {1.xx} Stephane Fillod:       X.25 support
 *980411 {1.34} Arnaldo Carvalho        i18n: catgets -> gnu gettext, substitution
 *                                      of sprintf for snprintf
 *10/1998	Andi Kleen              Use new interface primitives.
 *990101 {1.36}	Bernd Eckenfels		usage updated to include -s and -C -F,
 *					fixed netstat -rC output (lib/inet_gr.c)
 *					removed broken NETLINK Support
 *					fixed format for /proc/net/udp|tcp|raw
 *					added -w,-t,-u TcpExt support to -s
 *990131 {1.37} Jan Kratochvil          added -p for prg_cache() & friends
 *                                      Flames to <short@ucw.cz>.
 *              Tuan Hoang              added IGMP support for IPv4 and IPv6
 *
 *990420 {1.38} Tuan Hoang              removed a useless assignment from igmp_do_one()
 *20010404 {1.39} Arnaldo Carvalho de Melo - use setlocale
 *20081201 {1.42} Brian Micek           added -L|--udplite options for RFC 3828 
 *20020722 {1.51} Thomas Preusser       added SCTP over IPv4 support
 *20130426        Heikki Hannikainen    added SCTP over IPv6 support
 *                Ivan Skytte Joergensen SCTP statistics, state display, man page updates
 *
 *              This program is free software; you can redistribute it
 *              and/or  modify it under  the terms of  the GNU General
 *              Public  License as  published  by  the  Free  Software
 *              Foundation;  either  version 2 of the License, or  (at
 *              your option) any later version.
 *
 */
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <ctype.h>
#include <fcntl.h>
#include <netdb.h>
#include <paths.h>
#include <pwd.h>
#include <getopt.h>
#include <sys/param.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <net/if.h>
#include <dirent.h>

#if HAVE_SELINUX
#include <selinux/selinux.h>
#endif
#include "net-support.h"
#include "pathnames.h"
#include "version.h"
#include "config.h"
#include "intl.h"
#include "sockets.h"
#include "interface.h"
#include "util.h"
#include "proc.h"

#if HAVE_AFBLUETOOTH
#include <bluetooth/bluetooth.h>
#endif

#define PROGNAME_WIDTH 20
#define SELINUX_WIDTH 50

#if !defined(s6_addr32) && defined(in6a_words)
#define s6_addr32 in6a_words	/* libinet6			*/
#endif

/* prototypes for statistics.c */
void parsesnmp(int, int, int, int);
void inittab(void);
void parsesnmp6(int, int, int);
void inittab6(void);

typedef enum {
    SS_FREE = 0,		/* not allocated                */
    SS_UNCONNECTED,		/* unconnected to any socket    */
    SS_CONNECTING,		/* in process of connecting     */
    SS_CONNECTED,		/* connected to socket          */
    SS_DISCONNECTING		/* in process of disconnecting  */
} socket_state;

#define SO_ACCEPTCON    (1<<16)	/* performed a listen           */
#define SO_WAITDATA     (1<<17)	/* wait data to read            */
#define SO_NOSPACE      (1<<18)	/* no space to write            */

#define DFLT_AF "inet"

#define FEATURE_NETSTAT
#include "lib/net-features.h"

static char *Release = RELEASE, *Signature = "Fred Baumgarten, Alan Cox, Bernd Eckenfels, Phil Blundell, Tuan Hoang, Brian Micek and others";


#define E_READ  -1
#define E_IOCTL -3

int flag_int = 0;
int flag_rou = 0;
int flag_mas = 0;
int flag_sta = 0;

int flag_all = 0;
int flag_lst = 0;
int flag_cnt = 0;
int flag_deb = 0;
int flag_not = 0;
int flag_cf  = 0;
int flag_opt = 0;
int flag_raw = 0;
int flag_tcp = 0;
int flag_sctp= 0;
int flag_udp = 0;
int flag_udplite = 0;
int flag_igmp= 0;
int flag_rom = 0;
int flag_exp = 1;
int flag_wide= 0;
int flag_prg = 0;
int flag_arg = 0;
int flag_ver = 0;
int flag_l2cap = 0;
int flag_rfcomm = 0;
int flag_selinux = 0;

FILE *procinfo;

#define INFO_GUTS1(file,name,proc,prot)			\
  procinfo = proc_fopen((file));			\
  if (procinfo == NULL) {				\
    if (errno != ENOENT) {				\
      perror((file));					\
      return -1;					\
    }							\
    if (flag_arg || flag_ver)				\
      ESYSNOT("netstat", (name));			\
    if (flag_arg)					\
      rc = 1;						\
  } else {						\
    do {						\
      if (fgets(buffer, sizeof(buffer), procinfo))	\
        (proc)(lnr++, buffer,prot);			\
    } while (!feof(procinfo));				\
    fclose(procinfo);					\
  }

#if HAVE_AFINET6
#define INFO_GUTS2(file,proc,prot)			\
  lnr = 0;						\
  procinfo = proc_fopen((file));		       	\
  if (procinfo != NULL) {				\
    do {						\
      if (fgets(buffer, sizeof(buffer), procinfo))	\
	(proc)(lnr++, buffer,prot);			\
    } while (!feof(procinfo));				\
    fclose(procinfo);					\
  }
#else
#define INFO_GUTS2(file,proc,prot)
#endif

#define INFO_GUTS3					\
 return rc;

#define INFO_GUTS6(file,file6,name,proc,prot4,prot6)	\
 char buffer[8192];					\
 int rc = 0;						\
 int lnr = 0;						\
 if (!flag_arg || flag_inet) {				\
    INFO_GUTS1(file,name,proc,prot4)			\
 }							\
 if (!flag_arg || flag_inet6) {				\
    INFO_GUTS2(file6,proc,prot6)			\
 }							\
 INFO_GUTS3

#define INFO_GUTS(file,name,proc,prot)			\
 char buffer[8192];					\
 int rc = 0;						\
 int lnr = 0;						\
 INFO_GUTS1(file,name,proc,prot)			\
 INFO_GUTS3

#define PROGNAME_WIDTHs PROGNAME_WIDTH1(PROGNAME_WIDTH)
#define PROGNAME_WIDTH1(s) PROGNAME_WIDTH2(s)
#define PROGNAME_WIDTH2(s) #s

#define SELINUX_WIDTHs SELINUX_WIDTH1(SELINUX_WIDTH)
#define SELINUX_WIDTH1(s) SELINUX_WIDTH2(s)
#define SELINUX_WIDTH2(s) #s

#define PRG_HASH_SIZE 211

static struct prg_node {
    struct prg_node *next;
    unsigned long inode;
    char name[PROGNAME_WIDTH];
    char scon[SELINUX_WIDTH];
} *prg_hash[PRG_HASH_SIZE];

static char prg_cache_loaded = 0;

#define PRG_HASHIT(x) ((x) % PRG_HASH_SIZE)

#define PROGNAME_BANNER "PID/Program name"
#define SELINUX_BANNER "Security Context"

#define print_progname_banner() do { if (flag_prg) printf(" %-" PROGNAME_WIDTHs "s",PROGNAME_BANNER); } while (0)

#define print_selinux_banner() do { if (flag_selinux) printf("%-" SELINUX_WIDTHs "s"," " SELINUX_BANNER); } while (0)

#define PRG_LOCAL_ADDRESS "local_address"
#define PRG_INODE	 "inode"
#define PRG_SOCKET_PFX    "socket:["
#define PRG_SOCKET_PFXl (strlen(PRG_SOCKET_PFX))
#define PRG_SOCKET_PFX2   "[0000]:"
#define PRG_SOCKET_PFX2l  (strlen(PRG_SOCKET_PFX2))


#ifndef LINE_MAX
#define LINE_MAX 4096
#endif

#define PATH_PROC	   "/proc"
#define PATH_FD_SUFF	"fd"
#define PATH_FD_SUFFl       strlen(PATH_FD_SUFF)
#define PATH_PROC_X_FD      PATH_PROC "/%s/" PATH_FD_SUFF
#define PATH_CMDLINE	"cmdline"
#define PATH_CMDLINEl       strlen(PATH_CMDLINE)

static void prg_cache_add(unsigned long inode, char *name, const char *scon)
{
    unsigned hi = PRG_HASHIT(inode);
    struct prg_node **pnp,*pn;

    prg_cache_loaded = 2;
    for (pnp = prg_hash + hi; (pn = *pnp); pnp = &pn->next) {
	if (pn->inode == inode) {
	    /* Some warning should be appropriate here
	       as we got multiple processes for one i-node */
	    return;
	}
    }
    if (!(*pnp = malloc(sizeof(**pnp)))) 
	return;
    pn = *pnp;
    pn->next = NULL;
    pn->inode = inode;
    if (strlen(name) > sizeof(pn->name) - 1) 
	name[sizeof(pn->name) - 1] = '\0';
    strcpy(pn->name, name);

    {
	int len = (strlen(scon) - sizeof(pn->scon)) + 1;
	if (len > 0) 
            strcpy(pn->scon, &scon[len + 1]);
	else
            strcpy(pn->scon, scon);
    }

}

static const char *prg_cache_get(unsigned long inode)
{
    unsigned hi = PRG_HASHIT(inode);
    struct prg_node *pn;

    for (pn = prg_hash[hi]; pn; pn = pn->next)
	if (pn->inode == inode)
	    return (pn->name);
    return ("-");
}

static const char *prg_cache_get_con(unsigned long inode)
{
    unsigned hi = PRG_HASHIT(inode);
    struct prg_node *pn;

    for (pn = prg_hash[hi]; pn; pn = pn->next)
	if (pn->inode == inode)
	    return (pn->scon);
    return ("-");
}

static void prg_cache_clear(void)
{
    struct prg_node **pnp,*pn;

    if (prg_cache_loaded == 2)
	for (pnp = prg_hash; pnp < prg_hash + PRG_HASH_SIZE; pnp++)
	    while ((pn = *pnp)) {
		*pnp = pn->next;
		free(pn);
	    }
    prg_cache_loaded = 0;
}

static void wait_continous(void)
{
    fflush(stdout);
    sleep(1);
}

static int extract_type_1_socket_inode(const char lname[], unsigned long * inode_p) {

    /* If lname is of the form "socket:[12345]", extract the "12345"
       as *inode_p.  Otherwise, return -1 as *inode_p.
       */

    if (strlen(lname) < PRG_SOCKET_PFXl+3) return(-1);
    
    if (memcmp(lname, PRG_SOCKET_PFX, PRG_SOCKET_PFXl)) return(-1);
    if (lname[strlen(lname)-1] != ']') return(-1);

    {
        char inode_str[strlen(lname + 1)];  /* e.g. "12345" */
        const int inode_str_len = strlen(lname) - PRG_SOCKET_PFXl - 1;
        char *serr;

        strncpy(inode_str, lname+PRG_SOCKET_PFXl, inode_str_len);
        inode_str[inode_str_len] = '\0';
        *inode_p = strtoul(inode_str, &serr, 0);
        if (!serr || *serr || *inode_p == ~0)
            return(-1);
    }
    return(0);
}



static int extract_type_2_socket_inode(const char lname[], unsigned long * inode_p) {

    /* If lname is of the form "[0000]:12345", extract the "12345"
       as *inode_p.  Otherwise, return -1 as *inode_p.
       */

    if (strlen(lname) < PRG_SOCKET_PFX2l+1) return(-1);
    if (memcmp(lname, PRG_SOCKET_PFX2, PRG_SOCKET_PFX2l)) return(-1);

    {
        char *serr;

        *inode_p = strtoul(lname + PRG_SOCKET_PFX2l, &serr, 0);
        if (!serr || *serr || *inode_p == ~0)
            return(-1);
    }
    return(0);
}




static void prg_cache_load(void)
{
    char line[LINE_MAX], eacces=0;
    int procfdlen, fd, cmdllen, lnamelen;
    char lname[30], cmdlbuf[512], finbuf[PROGNAME_WIDTH];
    unsigned long inode;
    const char *cs, *cmdlp;
    DIR *dirproc = NULL, *dirfd = NULL;
    struct dirent *direproc, *direfd;
#if HAVE_SELINUX
    security_context_t scon = NULL;
#endif

    if (prg_cache_loaded || !flag_prg) return;
    prg_cache_loaded = 1;
    cmdlbuf[sizeof(cmdlbuf) - 1] = '\0';
    if (!(dirproc=opendir(PATH_PROC))) goto fail;
    while (errno = 0, direproc = readdir(dirproc)) {
	for (cs = direproc->d_name; *cs; cs++)
	    if (!isdigit(*cs)) 
		break;
	if (*cs) 
	    continue;
	procfdlen = snprintf(line,sizeof(line),PATH_PROC_X_FD,direproc->d_name);
	if (procfdlen <= 0 || procfdlen >= sizeof(line) - 5) 
	    continue;
	errno = 0;
	dirfd = opendir(line);
	if (! dirfd) {
	    if (errno == EACCES) 
		eacces = 1;
	    continue;
	}
	line[procfdlen] = '/';
	cmdlp = NULL;
	while ((direfd = readdir(dirfd))) {
           /* Skip . and .. */
           if (!isdigit(direfd->d_name[0]))
               continue;
	    if (procfdlen + 1 + strlen(direfd->d_name) + 1 > sizeof(line)) 
		continue;
	    memcpy(line + procfdlen - PATH_FD_SUFFl, PATH_FD_SUFF "/",
		   PATH_FD_SUFFl + 1);
	    strcpy(line + procfdlen + 1, direfd->d_name);
	    lnamelen = readlink(line, lname, sizeof(lname) - 1);
	    if (lnamelen == -1)
		    continue;
            lname[lnamelen] = '\0';  /*make it a null-terminated string*/

            if (extract_type_1_socket_inode(lname, &inode) < 0)
              if (extract_type_2_socket_inode(lname, &inode) < 0)
                continue;

	    if (!cmdlp) {
		if (procfdlen - PATH_FD_SUFFl + PATH_CMDLINEl >= 
		    sizeof(line) - 5) 
		    continue;
		strcpy(line + procfdlen-PATH_FD_SUFFl, PATH_CMDLINE);
		fd = open(line, O_RDONLY);
		if (fd < 0) 
		    continue;
		cmdllen = read(fd, cmdlbuf, sizeof(cmdlbuf) - 1);
		if (close(fd)) 
		    continue;
		if (cmdllen == -1) 
		    continue;
		if (cmdllen < sizeof(cmdlbuf) - 1) 
		    cmdlbuf[cmdllen]='\0';
		if (cmdlbuf[0] == '/' && (cmdlp = strrchr(cmdlbuf, '/'))) 
		    cmdlp++;
		else 
		    cmdlp = cmdlbuf;
	    }

	    snprintf(finbuf, sizeof(finbuf), "%s/%s", direproc->d_name, cmdlp);
#if HAVE_SELINUX
	    if (getpidcon(atoi(direproc->d_name), &scon) == -1) {
		    scon=strdup("-");
	    }
	    prg_cache_add(inode, finbuf, scon);
	    freecon(scon);
#else
	    prg_cache_add(inode, finbuf, "-");
#endif
	}
	closedir(dirfd); 
	dirfd = NULL;
    }
    if (dirproc) 
	closedir(dirproc);
    if (dirfd) 
	closedir(dirfd);
    if (!eacces) 
	return;
    if (prg_cache_loaded == 1) {
    fail:
	fprintf(stderr,_("(No info could be read for \"-p\": geteuid()=%d but you should be root.)\n"),
		geteuid());
    }
    else
	fprintf(stderr, _("(Not all processes could be identified, non-owned process info\n"
			 " will not be shown, you would have to be root to see it all.)\n"));
}

#if HAVE_AFNETROM
static const char *netrom_state[] =
{
    N_("LISTENING"),
    N_("CONN SENT"),
    N_("DISC SENT"),
    N_("ESTABLISHED")
};

static int netrom_info(void)
{
    FILE *f;
    char buffer[256], dev[16];
    int st, vs, vr, sendq, recvq, ret;

    f = proc_fopen(_PATH_PROCNET_NR);
    if (f == NULL) {
	if (errno != ENOENT) {
	    perror(_PATH_PROCNET_NR);
	    return (-1);
	}
	if (flag_arg || flag_ver)
	    ESYSNOT("netstat", "AF NETROM");
	if (flag_arg)
	    return (1);
	else
	    return (0);
    }
    printf(_("Active NET/ROM sockets\n"));
    printf(_("User       Dest       Source     Device  State        Vr/Vs    Send-Q  Recv-Q\n"));
    if (fgets(buffer, 256, f))
	/* eat line */;

    while (fgets(buffer, 256, f)) {
	buffer[9] = 0;
	buffer[19] = 0;
	buffer[29] = 0;
	ret = sscanf(buffer + 30, "%s %*x/%*x %*x/%*x %d %d %d %*d %*d/%*d %*d/%*d %*d/%*d %*d/%*d %*d/%*d %*d %d %d %*d",
	       dev, &st, &vs, &vr, &sendq, &recvq);
	if (ret != 6) {
	    printf(_("Problem reading data from %s\n"), _PATH_PROCNET_NR);
	    continue;
	}
	printf("%-9s  %-9s  %-9s  %-6s  %-11s  %03d/%03d  %-6d  %-6d\n",
	       buffer, buffer + 10, buffer + 20,
	       dev,
	       _(netrom_state[st]),
	       vr, vs, sendq, recvq);
    }
    fclose(f);
    return 0;
}
#endif

/* These enums are used by IPX too. :-( */
enum {
    TCP_ESTABLISHED = 1,
    TCP_SYN_SENT,
    TCP_SYN_RECV,
    TCP_FIN_WAIT1,
    TCP_FIN_WAIT2,
    TCP_TIME_WAIT,
    TCP_CLOSE,
    TCP_CLOSE_WAIT,
    TCP_LAST_ACK,
    TCP_LISTEN,
    TCP_CLOSING			/* now a valid state */
};

#if HAVE_AFINET || HAVE_AFINET6

#define TCP_NSTATES 12

static const char *tcp_state[] =
{
    "",
    N_("ESTABLISHED"),
    N_("SYN_SENT"),
    N_("SYN_RECV"),
    N_("FIN_WAIT1"),
    N_("FIN_WAIT2"),
    N_("TIME_WAIT"),
    N_("CLOSE"),
    N_("CLOSE_WAIT"),
    N_("LAST_ACK"),
    N_("LISTEN"),
    N_("CLOSING")
};

static void finish_this_one(int uid, unsigned long inode, const char *timers)
{
    struct passwd *pw;

    if (flag_exp > 1) {
	if (!(flag_not & FLAG_NUM_USER) && ((pw = getpwuid(uid)) != NULL))
	    printf(" %-10s ", pw->pw_name);
	else
	    printf(" %-10d ", uid);
	printf("%-10lu",inode);
    }
    if (flag_prg)
	printf(" %-" PROGNAME_WIDTHs "s",prg_cache_get(inode));
    if (flag_selinux)
	printf(" %-" SELINUX_WIDTHs "s",prg_cache_get_con(inode));

    if (flag_opt)
	printf(" %s", timers);
    putchar('\n');
}

static void igmp_do_one(int lnr, const char *line,const char *prot)
{
    char mcast_addr[128];
#if HAVE_AFINET6
    struct sockaddr_in6 mcastaddr;
    char addr6[INET6_ADDRSTRLEN];
    struct in6_addr in6;
    extern struct aftype inet6_aftype;
#else
    struct sockaddr_in mcastaddr;
#endif
    struct aftype *ap;
    static int idx_flag = 0;
    static int igmp6_flag = 0;
    static char device[16];
    int num, idx, refcnt;

    if (lnr == 0) {
	/* IPV6 ONLY */
	/* igmp6 file does not have any comments on first line */
	if ( strstr( line, "Device" ) == NULL ) {
	    igmp6_flag = 1;
	} else {
	    /* IPV4 ONLY */
	    /* 2.1.x kernels and up have Idx field */
	    /* 2.0.x and below do not have Idx field */
	    if ( strncmp( line, "Idx", strlen("Idx") ) == 0 )
		idx_flag = 1;
	    else
		idx_flag = 0;
	    return;
	}
    }

    if (igmp6_flag) {    /* IPV6 */
#if HAVE_AFINET6
	num = sscanf( line, "%d %15s %64[0-9A-Fa-f] %d", &idx, device, mcast_addr, &refcnt );
	if (num == 4) {
	    /* Demangle what the kernel gives us */
	    sscanf(mcast_addr, "%08X%08X%08X%08X",
		   &in6.s6_addr32[0], &in6.s6_addr32[1],
           &in6.s6_addr32[2], &in6.s6_addr32[3]);
	    in6.s6_addr32[0] = htonl(in6.s6_addr32[0]);
	    in6.s6_addr32[1] = htonl(in6.s6_addr32[1]);
	    in6.s6_addr32[2] = htonl(in6.s6_addr32[2]);
	    in6.s6_addr32[3] = htonl(in6.s6_addr32[3]);
        inet_ntop(AF_INET6, &in6, addr6, sizeof(addr6));
	    inet6_aftype.input(1, addr6, (struct sockaddr *) &mcastaddr);
	    mcastaddr.sin6_family = AF_INET6;
	} else {
	    fprintf(stderr, _("warning, got bogus igmp6 line %d.\n"), lnr);
	    return;
	}

	if ((ap = get_afntype(((struct sockaddr *) &mcastaddr)->sa_family)) == NULL) {
	    fprintf(stderr, _("netstat: unsupported address family %d !\n"),
		    ((struct sockaddr *) &mcastaddr)->sa_family);
	    return;
	}
	safe_strncpy(mcast_addr, ap->sprint((struct sockaddr *) &mcastaddr, 
				      flag_not & FLAG_NUM_HOST), sizeof(mcast_addr));
	printf("%-15s %-6d %s\n", device, refcnt, mcast_addr);
#endif
    } else {    /* IPV4 */
#if HAVE_AFINET
	if (line[0] != '\t') {
	    if (idx_flag) {
		if ((num = sscanf( line, "%d\t%10c", &idx, device)) < 2) {
		    fprintf(stderr, _("warning, got bogus igmp line %d.\n"), lnr);
		    return;
		}
	    } else {
		if ( (num = sscanf( line, "%10c", device )) < 1 ) {
		    fprintf(stderr, _("warning, got bogus igmp line %d.\n"), lnr);
		    return;
		}
	    }
	    device[10] = '\0';
	    return;
	} else if ( line[0] == '\t' ) {
	    if ( (num = sscanf(line, "\t%8[0-9A-Fa-f] %d", mcast_addr, &refcnt)) < 2 ) {
		fprintf(stderr, _("warning, got bogus igmp line %d.\n"), lnr);
		return;
	    }
	    sscanf( mcast_addr, "%X",
		    &((struct sockaddr_in *) &mcastaddr)->sin_addr.s_addr );
	    ((struct sockaddr *) &mcastaddr)->sa_family = AF_INET;
	} else {
	    fprintf(stderr, _("warning, got bogus igmp line %d.\n"), lnr);
	    return;
	}
	
	if ((ap = get_afntype(((struct sockaddr *) &mcastaddr)->sa_family)) == NULL) {
	    fprintf(stderr, _("netstat: unsupported address family %d !\n"),
		    ((struct sockaddr *) &mcastaddr)->sa_family);
	    return;
	}
	safe_strncpy(mcast_addr, ap->sprint((struct sockaddr *) &mcastaddr, 
				      flag_not & FLAG_NUM_HOST), sizeof(mcast_addr));
	printf("%-15s %-6d %s\n", device, refcnt, mcast_addr );
#endif
    }    /* IPV4 */
}

#if HAVE_AFX25
static int x25_info(void)
{
       FILE *f=proc_fopen(_PATH_PROCNET_X25);
       char buffer[256],dev[16];
       int st,vs,vr,sendq,recvq,lci;
       static char *x25_state[5]=
       {
               "LISTENING",
               "SABM_SENT",
               "DISC_SENT",
               "ESTABLISHED",
               "RECOVERY"
       };
       if(!f)
       {
               if (errno != ENOENT) {
                       perror(_PATH_PROCNET_X25);
                       return(-1);
               }
               if (flag_arg || flag_ver)
                       ESYSNOT("netstat","AF X25");
               if (flag_arg)
                       return(1);
               else
                       return(0);
       }
       printf( _("Active X.25 sockets\n"));
       /* IMHO, Vr/Vs is not very usefull --SF */
       printf( _("Dest         Source          Device  LCI  State        Vr/Vs  Send-Q  Recv-Q\n"));
       if (fgets(buffer,256,f))
               /* eat line */;
       while(fgets(buffer,256,f))
       {
               buffer[10]=0;
               buffer[20]=0;
               sscanf(buffer+22,"%s %d %d %d %d %*d %*d %*d %*d %*d %*d %d %d %*d",
                       dev,&lci,&st,&vs,&vr,&sendq,&recvq);
               if (!(flag_all || lci))
                       continue;
               printf("%-15s %-15s %-7s %-3d  %-11s  %02d/%02d  %-6d  %-6d\n",
                       buffer,buffer+11,
                       dev,
                       lci,
                       x25_state[st],
                       vr,vs,sendq,recvq);
       }
       fclose(f);
       return 0;               
}
#endif

static int igmp_info(void)
{
    INFO_GUTS6(_PATH_PROCNET_IGMP, _PATH_PROCNET_IGMP6, "AF INET (igmp)",
	       igmp_do_one, "igmp", "igmp6");
}

static void addr_do_one(char *buf, size_t buf_len, size_t short_len, struct aftype *ap,
#if HAVE_AFINET6
			struct sockaddr_in6 *addr,
#else
			struct sockaddr_in *addr,
#endif
			int port, const char *proto
)
{
    const char *sport, *saddr;
    size_t port_len, addr_len;

    saddr = ap->sprint((struct sockaddr *)addr, flag_not & FLAG_NUM_HOST);
    sport = get_sname(htons(port), proto, flag_not & FLAG_NUM_PORT);
    addr_len = strlen(saddr);
    port_len = strlen(sport);
    if (!flag_wide && (addr_len + port_len > short_len)) {
        /* Assume port name is short */
        port_len = netmin(port_len, short_len - 4);
        addr_len = short_len - port_len;
        strncpy(buf, saddr, addr_len);
        buf[addr_len] = '\0';
        strcat(buf, ":");
        strncat(buf, sport, port_len);
    } else
        snprintf(buf, buf_len, "%s:%s", saddr, sport);
}

/*
 *	SCTP support
 */


#define SCTP_NSTATES  9         /* The number of states in array */

static const char *sctp_state[] = {
    N_("EMPTY"),
    N_("CLOSED"),
    N_("COOKIE_WAIT"),
    N_("COOKIE_ECHOED"),
    N_("ESTABLISHED"),
    N_("SHUTDOWN_PENDING"),
    N_("SHUTDOWN_SENT"),
    N_("SHUTDOWN_RECEIVED"),
    N_("SHUTDOWN_ACK_SENT")
};

static const char *sctp_state_str(int state)
{
    if (state >= 0 && state < SCTP_NSTATES)
        return sctp_state[state];
    else {
        static char state_str_buf[64];
        sprintf(state_str_buf, "UNKNOWN(%d)", state);
        return state_str_buf;
    }
}

static const char *sctp_socket_state_str(int state)
{
    if (state >= 0 && state < TCP_NSTATES)
        return tcp_state[state];
    else {
        static char state_str_buf[64];
        sprintf(state_str_buf, "UNKNOWN(%d)", state);
        return state_str_buf;
    }
}

union sockaddr_u {
    struct sockaddr     sa;
    struct sockaddr_in  si;
#if HAVE_AFINET6
    struct sockaddr_in6 si6;
#endif
};

static int ip_parse_dots(union sockaddr_u *addr, char const *src)
{
    /* SCTP sockets can use IPv6 and IPv4 addresses, at the same time, for
     * the same socket. Try to parse an IPv4 address first.
     */
    unsigned  a, b, c, d;
    unsigned  el = sscanf(src, "%u.%u.%u.%u", &a, &b, &c, &d);
    
    if (el == 4) {
        addr->sa.sa_family = AF_INET;
        addr->si.sin_addr.s_addr = htonl((a << 24)|(b << 16)|(c << 8)|d);
        return 0;
    }

#if HAVE_AFINET6
    /* check if it looks right first */
    el = sscanf(src, "%04X:%04X:%04X:%04X:%04X:%04X:%04X:%04X ",
           &a, &a, &a, &a, &a, &a, &a, &a);
    
    if (el == 8) {
        addr->sa.sa_family = AF_INET6;
        
        if (inet_pton(AF_INET6, src, (void *)&addr->si6.sin6_addr) != 1)
            return -1;
        
        return 0;
    }
#endif
    
    return -1;
}

static void print_ip_service(union sockaddr_u *addr, char const *protname,
			     char *buf, unsigned size)
{
    struct aftype *ap;
    
    if (size == 0)
        return;
    
    /* print host */
    if ((ap = get_afntype(addr->sa.sa_family)) == NULL) {
        fprintf(stderr, _("netstat: unsupported address family %d !\n"),
            addr->sa.sa_family);
        return;
    }
    
    return addr_do_one(buf, size, 22, ap,
#if HAVE_AFINET6
            &addr->si6,
#else
            &addr->si,
#endif
            ntohs(addr->si.sin_port), "sctp");
}

/* process single SCTP endpoint */
static void sctp_do_ept(int lnr, char *line, const char *prot)
{
  union sockaddr_u laddr, raddr;
  unsigned         sstate, uid, inode;
  
  memset((void *)&raddr, 0, sizeof(raddr));

  char l_addr[23] = "";
  char r_addr[23] = "";

  /* fill sockaddr_in structures */
  {
    unsigned  lport;
    unsigned  ate;
    char *addr, *s;

    if(lnr == 0)  return;
    
    // ENDPT     SOCK   STY SST HBKT LPORT   UID INODE LADDRS
    if (sscanf(line, "%*X %*X %*u %u %*u %u %u %u %n",
        &sstate, &lport, &uid, &inode, &ate) < 3)  goto err;

    /* decode IP address */
    addr = line + ate;
    s = addr;
    while (*s != 0 && !isspace(*s))
        s++;
    if (*s == 0)
        goto err;
    *s = 0;
    
    if (ip_parse_dots(&laddr, addr))
        goto err;
    
    laddr.si.sin_port = htons(lport);
    raddr.sa.sa_family = laddr.sa.sa_family;
  }

  /* print IP:service to l_addr and r_addr */
  print_ip_service(&laddr, prot, l_addr, sizeof(l_addr));
  print_ip_service(&raddr, prot, r_addr, sizeof(r_addr));

  /* Print line */
  printf("%-4s  %6d %6d %-*s %-*s %-11s",
	 prot, 0, 0,
	 (int)netmax(23,strlen(l_addr)), l_addr,
	 (int)netmax(23,strlen(r_addr)), r_addr,
	 sctp_socket_state_str(sstate));
  finish_this_one(uid, inode, "");
  
  return;
 err:
  fprintf(stderr, "Warning: SCTP endpoint parsing failed on line: %d\n", lnr);
}

static char *sctp_next_nonprimary(char *s, char **end)
{
    while (*s) {
       while (*s == ' ')
           s++;
       
       char *start = s;
       
       while (*s && *s != ' ')
           s++;
       
       if (*start != '*') {
           *end = s;
           return start;
       }
    }
    
    return s;
}

static char *sctp_addr(char *dst, int dlen, char *src, int slen)
{
    union sockaddr_u addr;
    struct aftype *ap;
    char addrs[42];
    
    dst[0] = 0;
    
    if (slen <= 0)
        return NULL;
    
    if (slen >= sizeof(addrs)-1)
        return NULL;
    
    safe_strncpy(addrs, src, slen + 1);
    
    if (ip_parse_dots(&addr, addrs) != 0)
        return NULL;
    
    if (!(ap = get_afntype(addr.sa.sa_family)))
        return NULL;
    
    if (dlen > 24)
        dlen = 24;
    
    safe_strncpy(dst, ap->sprint((struct sockaddr *)&addr, flag_not), dlen);
    
    return dst;
}

/* process single SCTP association */
static void sctp_do_assoc(int lnr, char *line, const char *prot)
{
  union sockaddr_u    laddr, raddr;
  unsigned long       rxq, txq;
  unsigned            state, sstate, uid, inode;
  char                l_addr[64], r_addr[64];
  char                *l_addrs, *r_addrs;

  /* fill sockaddr_in structures */
  {
    unsigned    lport, rport;
    unsigned    ate;
    char *addr;
    char *s;

    if (lnr == 0)
        return;
    
    //  ASSOC     SOCK   STY SST ST HBKT ASSOC-ID TX_QUEUE RX_QUEUE UID INODE LPORT RPORT LADDRS <-> RADDRS HBINT INS OUTS MAXRT T1X T2X RTXC
    if (sscanf(line, "%*X %*X %*u %u %u %*u %*u %lu %lu %u %u %u %u %n",
    	      &sstate, &state, &txq, &rxq, &uid, &inode, &lport, &rport, &ate) < 6)  goto err;

    /* decode IP addresses */
    
    /* split out local and remote address sets */
    l_addrs = line + ate;
    r_addrs = strstr(l_addrs, "<->");
    if (!r_addrs) goto err;
    *r_addrs = 0;
    r_addrs += 3;
    s = strchr(r_addrs, '\t');
    if (!s) goto err;
    *s = 0;
    
    /* find primary local address */
    addr = strchr(l_addrs, '*');
    if (addr != 0)
        addr++;
    else
    {
        /* no primary address marked, use first address and skip it */
        addr = l_addrs;
        l_addrs = strchr(l_addrs, ' ');
        if (!l_addrs)
            goto err;
    }

    s = strchr(addr, ' ');
    if (!s) goto err;
    if (s-addr+1 > sizeof(l_addr))
        goto err;
        
    safe_strncpy(l_addr, addr, s-addr+1);
    
    if (ip_parse_dots(&laddr, l_addr))
        goto err;
    
    /* find primary remote address */
    addr = strchr(r_addrs, '*');
    if (addr == 0) goto err;
    
    addr++;
    s = strchr(addr, ' ');
    if (!s) goto err;
    if (s-addr+1 > sizeof(r_addr))
        goto err;
    
    safe_strncpy(r_addr, addr, s-addr+1);
    
    if (ip_parse_dots(&raddr, r_addr))
        goto err;

    /* complete sockaddr_in structures, port is in the same location for both IPv4 and IPv6 */
    laddr.si.sin_port = htons(lport);
    raddr.si.sin_port = htons(rport);
  }

  /* print IP:service to l_addr and r_addr */
  print_ip_service(&laddr, prot, l_addr, sizeof(l_addr));
  print_ip_service(&raddr, prot, r_addr, sizeof(r_addr));
  
  /* Print line */
  printf("%-4s  %6ld %6ld %-*s %-*s %-11s",
	 prot, rxq, txq,
	 (int)netmax(23,strlen(l_addr)), l_addr,
	 (int)netmax(23,strlen(r_addr)), r_addr,
	 sctp_state_str(state));
  finish_this_one(uid, inode, "");
  
    if (flag_sctp > 1) {
        /* print non-primary addresses */
        while (*l_addrs || *r_addrs) {
            char *le = l_addrs;
            char *re = r_addrs;
            l_addrs = sctp_next_nonprimary(l_addrs, &le);
            r_addrs = sctp_next_nonprimary(r_addrs, &re);
            
            if (le == l_addrs && re == r_addrs)
                break;
            
            sctp_addr(l_addr, sizeof(l_addr), l_addrs, le-l_addrs);
            sctp_addr(r_addr, sizeof(r_addr), r_addrs, re-r_addrs);
            
            printf("                    %-*s %-*s\n", (int)netmax(23,strlen(l_addr)), l_addr, (int)netmax(23,strlen(r_addr)), r_addr);
            
            l_addrs = le;
            r_addrs = re;
        }
    }
  
  return;
 err:
  fprintf(stderr, "Warning: SCTP association parsing failed on line: %d\n", lnr);
}

static int sctp_info_epts(void) {
  INFO_GUTS(_PATH_PROCNET_SCTPEPTS, "AF INET (sctp)", sctp_do_ept, "sctp");
}

static int sctp_info_assocs(void) {
  INFO_GUTS(_PATH_PROCNET_SCTPASSOCS, "AF INET (sctp)", sctp_do_assoc, "sctp");
}

static int sctp_info(void) {
    int  res;
    
    if (flag_all || flag_lst) {
        res = sctp_info_epts();
        if (res)
            return res;
    }
    
    if (flag_lst && !flag_all)
        return 0;
        
    return sctp_info_assocs();
}



static void tcp_do_one(int lnr, const char *line, const char *prot)
{
    unsigned long rxq, txq, time_len, retr, inode;
    int num, local_port, rem_port, d, state, uid, timer_run, timeout;
    char rem_addr[128], local_addr[128], timers[64];
    struct aftype *ap;
#if HAVE_AFINET6
    struct sockaddr_in6 localaddr, remaddr;
    char addr6[INET6_ADDRSTRLEN];
    struct in6_addr in6;
    extern struct aftype inet6_aftype;
#else
    struct sockaddr_in localaddr, remaddr;
#endif

    if (lnr == 0)
	return;

    num = sscanf(line,
    "%d: %64[0-9A-Fa-f]:%X %64[0-9A-Fa-f]:%X %X %lX:%lX %X:%lX %lX %d %d %lu %*s\n",
		 &d, local_addr, &local_port, rem_addr, &rem_port, &state,
		 &txq, &rxq, &timer_run, &time_len, &retr, &uid, &timeout, &inode);

    if (num < 11) {
	fprintf(stderr, _("warning, got bogus tcp line.\n"));
	return;
    }

    if (!flag_all && ((flag_lst && rem_port) || (!flag_lst && !rem_port)))
      return;

    if (strlen(local_addr) > 8) {
#if HAVE_AFINET6
	/* Demangle what the kernel gives us */
	sscanf(local_addr, "%08X%08X%08X%08X",
	       &in6.s6_addr32[0], &in6.s6_addr32[1],
           &in6.s6_addr32[2], &in6.s6_addr32[3]);
	inet_ntop(AF_INET6, &in6, addr6, sizeof(addr6));
	inet6_aftype.input(1, addr6, (struct sockaddr *) &localaddr);
	sscanf(rem_addr, "%08X%08X%08X%08X",
	       &in6.s6_addr32[0], &in6.s6_addr32[1],
	       &in6.s6_addr32[2], &in6.s6_addr32[3]);
	inet_ntop(AF_INET6, &in6, addr6, sizeof(addr6));
	inet6_aftype.input(1, addr6, (struct sockaddr *) &remaddr);
	localaddr.sin6_family = AF_INET6;
	remaddr.sin6_family = AF_INET6;
#endif
    } else {
	sscanf(local_addr, "%X",
	       &((struct sockaddr_in *) &localaddr)->sin_addr.s_addr);
	sscanf(rem_addr, "%X",
	       &((struct sockaddr_in *) &remaddr)->sin_addr.s_addr);
	((struct sockaddr *) &localaddr)->sa_family = AF_INET;
	((struct sockaddr *) &remaddr)->sa_family = AF_INET;
    }

    if ((ap = get_afntype(((struct sockaddr *) &localaddr)->sa_family)) == NULL) {
	fprintf(stderr, _("netstat: unsupported address family %d !\n"),
		((struct sockaddr *) &localaddr)->sa_family);
	return;
    }

	addr_do_one(local_addr, sizeof(local_addr), 22, ap, &localaddr, local_port, "tcp");
	addr_do_one(rem_addr, sizeof(rem_addr), 22, ap, &remaddr, rem_port, "tcp");

	timers[0] = '\0';
	if (flag_opt)
	    switch (timer_run) {
	    case 0:
		snprintf(timers, sizeof(timers), _("off (0.00/%ld/%d)"), retr, timeout);
		break;

	    case 1:
		snprintf(timers, sizeof(timers), _("on (%2.2f/%ld/%d)"),
			 (double) time_len / HZ, retr, timeout);
		break;

	    case 2:
		snprintf(timers, sizeof(timers), _("keepalive (%2.2f/%ld/%d)"),
			 (double) time_len / HZ, retr, timeout);
		break;

	    case 3:
		snprintf(timers, sizeof(timers), _("timewait (%2.2f/%ld/%d)"),
			 (double) time_len / HZ, retr, timeout);
		break;

	    default:
		snprintf(timers, sizeof(timers), _("unkn-%d (%2.2f/%ld/%d)"),
			 timer_run, (double) time_len / HZ, retr, timeout);
		break;
	    }

	printf("%-4s  %6ld %6ld %-*s %-*s %-11s",
	       prot, rxq, txq, (int)netmax(23,strlen(local_addr)), local_addr, (int)netmax(23,strlen(rem_addr)), rem_addr, _(tcp_state[state]));

	finish_this_one(uid,inode,timers);
}

static int tcp_info(void)
{
    INFO_GUTS6(_PATH_PROCNET_TCP, _PATH_PROCNET_TCP6, "AF INET (tcp)",
	       tcp_do_one, "tcp", "tcp6");
}

static void udp_do_one(int lnr, const char *line,const char *prot)
{
    char local_addr[64], rem_addr[64];
    char *udp_state, timers[64];
    int num, local_port, rem_port, d, state, timer_run, uid, timeout;
#if HAVE_AFINET6
    struct sockaddr_in6 localaddr, remaddr;
    char addr6[INET6_ADDRSTRLEN];
    struct in6_addr in6;
    extern struct aftype inet6_aftype;
#else
    struct sockaddr_in localaddr, remaddr;
#endif
    struct aftype *ap;
    unsigned long rxq, txq, time_len, retr, inode;

    if (lnr == 0)
	return;

    num = sscanf(line,
		 "%d: %64[0-9A-Fa-f]:%X %64[0-9A-Fa-f]:%X %X %lX:%lX %X:%lX %lX %d %d %lu %*s\n",
		 &d, local_addr, &local_port,
		 rem_addr, &rem_port, &state,
	  &txq, &rxq, &timer_run, &time_len, &retr, &uid, &timeout, &inode);

    if (num < 10) {
	fprintf(stderr, _("warning, got bogus udp line.\n"));
	return;
    }

    if (strlen(local_addr) > 8) {
#if HAVE_AFINET6
	sscanf(local_addr, "%08X%08X%08X%08X",
	       &in6.s6_addr32[0], &in6.s6_addr32[1],
	       &in6.s6_addr32[2], &in6.s6_addr32[3]);
	inet_ntop(AF_INET6, &in6, addr6, sizeof(addr6));
	inet6_aftype.input(1, addr6, (struct sockaddr *) &localaddr);
	sscanf(rem_addr, "%08X%08X%08X%08X",
	       &in6.s6_addr32[0], &in6.s6_addr32[1],
	       &in6.s6_addr32[2], &in6.s6_addr32[3]);
	inet_ntop(AF_INET6, &in6, addr6, sizeof(addr6));
	inet6_aftype.input(1, addr6, (struct sockaddr *) &remaddr);
	localaddr.sin6_family = AF_INET6;
	remaddr.sin6_family = AF_INET6;
#endif
    } else {
	sscanf(local_addr, "%X",
	       &((struct sockaddr_in *) &localaddr)->sin_addr.s_addr);
	sscanf(rem_addr, "%X",
	       &((struct sockaddr_in *) &remaddr)->sin_addr.s_addr);
	((struct sockaddr *) &localaddr)->sa_family = AF_INET;
	((struct sockaddr *) &remaddr)->sa_family = AF_INET;
    }

    retr = 0L;

    if ((ap = get_afntype(((struct sockaddr *) &localaddr)->sa_family)) == NULL) {
	fprintf(stderr, _("netstat: unsupported address family %d !\n"),
		((struct sockaddr *) &localaddr)->sa_family);
	return;
    }
    switch (state) {
    case TCP_ESTABLISHED:
	udp_state = _("ESTABLISHED");
	break;

    case TCP_CLOSE:
	udp_state = "";
	break;

    default:
	udp_state = _("UNKNOWN");
	break;
    }

#if HAVE_AFINET6
#define notnull(A) (((A.sin6_family == AF_INET6) && \
	 ((A.sin6_addr.s6_addr32[0]) ||            \
	  (A.sin6_addr.s6_addr32[1]) ||            \
	  (A.sin6_addr.s6_addr32[2]) ||            \
	  (A.sin6_addr.s6_addr32[3]))) ||          \
	((A.sin6_family == AF_INET) &&             \
	 ((struct sockaddr_in *) &A)->sin_addr.s_addr))
#else
#define notnull(A) (A.sin_addr.s_addr)
#endif

    if (flag_all || (notnull(remaddr) && !flag_lst) || (!notnull(remaddr) && flag_lst))
    {
	addr_do_one(local_addr, sizeof(local_addr), 22, ap, &localaddr, local_port, "udp");
	addr_do_one(rem_addr, sizeof(rem_addr), 22, ap, &remaddr, rem_port, "udp");

	timers[0] = '\0';
	if (flag_opt)
	    switch (timer_run) {
	    case 0:
		snprintf(timers, sizeof(timers), _("off (0.00/%ld/%d)"), retr, timeout);
		break;

	    case 1:
	    case 2:
		snprintf(timers, sizeof(timers), _("on%d (%2.2f/%ld/%d)"), timer_run, (double) time_len / 100, retr, timeout);
		break;

	    default:
		snprintf(timers, sizeof(timers), _("unkn-%d (%2.2f/%ld/%d)"), timer_run, (double) time_len / 100,
			 retr, timeout);
		break;
	    }
	printf("%-5s %6ld %6ld %-23s %-23s %-11s",
	       prot, rxq, txq, local_addr, rem_addr, udp_state);

	finish_this_one(uid,inode,timers);
    }
}

static int udp_info(void)
{
    INFO_GUTS6(_PATH_PROCNET_UDP, _PATH_PROCNET_UDP6, "AF INET (udp)",
	       udp_do_one, "udp", "udp6");
}

static int udplite_info(void)
{
    INFO_GUTS6(_PATH_PROCNET_UDPLITE, _PATH_PROCNET_UDPLITE6, 
               "AF INET (udplite)", udp_do_one, "udpl", "udpl6" );
}

static void raw_do_one(int lnr, const char *line,const char *prot)
{
    char local_addr[64], rem_addr[64];
    char timers[64];
    int num, local_port, rem_port, d, state, timer_run, uid, timeout;
#if HAVE_AFINET6
    struct sockaddr_in6 localaddr, remaddr;
    char addr6[INET6_ADDRSTRLEN];
    struct in6_addr in6;
    extern struct aftype inet6_aftype;
#else
    struct sockaddr_in localaddr, remaddr;
#endif
    struct aftype *ap;
    unsigned long rxq, txq, time_len, retr, inode;

    if (lnr == 0)
	return;

    num = sscanf(line,
		 "%d: %64[0-9A-Fa-f]:%X %64[0-9A-Fa-f]:%X %X %lX:%lX %X:%lX %lX %d %d %lu %*s\n",
		 &d, local_addr, &local_port, rem_addr, &rem_port, &state,
	  &txq, &rxq, &timer_run, &time_len, &retr, &uid, &timeout, &inode);
	  
    if (num < 10) {
    	fprintf(stderr, _("warning, got bogus raw line.\n"));
	return;
    }

    if (strlen(local_addr) > 8) {
#if HAVE_AFINET6
	sscanf(local_addr, "%08X%08X%08X%08X",
	       &in6.s6_addr32[0], &in6.s6_addr32[1],
           &in6.s6_addr32[2], &in6.s6_addr32[3]);
    inet_ntop(AF_INET6, &in6, addr6, sizeof(addr6));
	inet6_aftype.input(1, addr6, (struct sockaddr *) &localaddr);
	sscanf(rem_addr, "%08X%08X%08X%08X",
	       &in6.s6_addr32[0], &in6.s6_addr32[1],
           &in6.s6_addr32[2], &in6.s6_addr32[3]);
    inet_ntop(AF_INET6, &in6, addr6, sizeof(addr6));
	inet6_aftype.input(1, addr6, (struct sockaddr *) &remaddr);
	localaddr.sin6_family = AF_INET6;
	remaddr.sin6_family = AF_INET6;
#endif
    } else {
	sscanf(local_addr, "%X",
	       &((struct sockaddr_in *) &localaddr)->sin_addr.s_addr);
	sscanf(rem_addr, "%X",
	       &((struct sockaddr_in *) &remaddr)->sin_addr.s_addr);
	((struct sockaddr *) &localaddr)->sa_family = AF_INET;
	((struct sockaddr *) &remaddr)->sa_family = AF_INET;
    }
#if HAVE_AFINET6
    if ((ap = get_afntype(localaddr.sin6_family)) == NULL) {
	fprintf(stderr, _("netstat: unsupported address family %d !\n"), localaddr.sin6_family);
	return;
    }
#else
    if ((ap = get_afntype(localaddr.sin_family)) == NULL) {
	fprintf(stderr, _("netstat: unsupported address family %d !\n"), localaddr.sin_family);
	return;
    }
#endif

    if (flag_all || (notnull(remaddr) && !flag_lst) || (!notnull(remaddr) && flag_lst))
    {
	addr_do_one(local_addr, sizeof(local_addr), 22, ap, &localaddr, local_port, "raw");
	addr_do_one(rem_addr, sizeof(rem_addr), 22, ap, &remaddr, rem_port, "raw");

	timers[0] = '\0';
	if (flag_opt)
	    switch (timer_run) {
	    case 0:
		snprintf(timers, sizeof(timers), _("off (0.00/%ld/%d)"), retr, timeout);
		break;

	    case 1:
            case 2:
		snprintf(timers, sizeof(timers), _("on%d (%2.2f/%ld/%d)"), timer_run, (double) time_len / 100,
			 retr, timeout);
		break;

	    default:
		snprintf(timers, sizeof(timers), _("unkn-%d (%2.2f/%ld/%d)"),
			 timer_run, (double) time_len / 100,
			 retr, timeout);
		break;
	    }
	printf("%-4s  %6ld %6ld %-23s %-23s %-11d",
	       prot, rxq, txq, local_addr, rem_addr, state);

	finish_this_one(uid,inode,timers);
    }
}

static int raw_info(void)
{
    INFO_GUTS6(_PATH_PROCNET_RAW, _PATH_PROCNET_RAW6, "AF INET (raw)",
	       raw_do_one, "raw", "raw6");
}

#endif


#if HAVE_AFUNIX

#define HAS_INODE 1

static void unix_do_one(int nr, const char *line, const char *prot)
{
    static int has = 0;
    char path[MAXPATHLEN], ss_flags[32];
    char *ss_proto, *ss_state, *ss_type;
    int num, state, type;
    void *d;
    unsigned long refcnt, proto, flags, inode;

    if (nr == 0) {
	if (strstr(line, "Inode"))
	    has |= HAS_INODE;
	return;
    }
    path[0] = '\0';
    num = sscanf(line, "%p: %lX %lX %lX %X %X %lu %s",
		 &d, &refcnt, &proto, &flags, &type, &state, &inode, path);
    if (num < 6) {
	fprintf(stderr, _("warning, got bogus unix line.\n"));
	return;
    }
    if (!(has & HAS_INODE))
	snprintf(path,sizeof(path),"%lu",inode);

    if (!flag_all) {
    	if ((state == SS_UNCONNECTED) && (flags & SO_ACCEPTCON)) {
    		if (!flag_lst)
    			return;
    	} else {
    		if (flag_lst)
    			return;
    	}
    }

    switch (proto) {
    case 0:
	ss_proto = "unix";
	break;

    default:
	ss_proto = "??";
    }

    switch (type) {
    case SOCK_STREAM:
	ss_type = _("STREAM");
	break;

    case SOCK_DGRAM:
	ss_type = _("DGRAM");
	break;

    case SOCK_RAW:
	ss_type = _("RAW");
	break;

    case SOCK_RDM:
	ss_type = _("RDM");
	break;

    case SOCK_SEQPACKET:
	ss_type = _("SEQPACKET");
	break;

    default:
	ss_type = _("UNKNOWN");
    }

    switch (state) {
    case SS_FREE:
	ss_state = _("FREE");
	break;

    case SS_UNCONNECTED:
	/*
	 * Unconnected sockets may be listening
	 * for something.
	 */
	if (flags & SO_ACCEPTCON) {
	    ss_state = _("LISTENING");
	} else {
	    ss_state = "";
	}
	break;

    case SS_CONNECTING:
	ss_state = _("CONNECTING");
	break;

    case SS_CONNECTED:
	ss_state = _("CONNECTED");
	break;

    case SS_DISCONNECTING:
	ss_state = _("DISCONNECTING");
	break;

    default:
	ss_state = _("UNKNOWN");
    }

    strcpy(ss_flags, "[ ");
    if (flags & SO_ACCEPTCON)
	strcat(ss_flags, "ACC ");
    if (flags & SO_WAITDATA)
	strcat(ss_flags, "W ");
    if (flags & SO_NOSPACE)
	strcat(ss_flags, "N ");

    strcat(ss_flags, "]");

    printf("%-5s %-6ld %-11s %-10s %-13s ",
	   ss_proto, refcnt, ss_flags, ss_type, ss_state);
    if (has & HAS_INODE)
	printf("%-8lu",inode);
    else
	printf("-       ");
    if (flag_prg)
	printf(" %-" PROGNAME_WIDTHs "s",(has & HAS_INODE?prg_cache_get(inode):"-"));
    if (flag_selinux)
	printf(" %-" SELINUX_WIDTHs "s",(has & HAS_INODE?prg_cache_get_con(inode):"-"));

    printf(" %s\n", path);
}

static int unix_info(void)
{

    printf(_("Active UNIX domain sockets "));
    if (flag_all)
	printf(_("(servers and established)"));
    else {
      if (flag_lst)
	printf(_("(only servers)"));
      else
	printf(_("(w/o servers)"));
    }

    printf(_("\nProto RefCnt Flags       Type       State         I-Node  "));
    print_progname_banner();
    print_selinux_banner();
    printf(_(" Path\n"));	/* xxx */

    {
	INFO_GUTS(_PATH_PROCNET_UNIX, "AF UNIX", unix_do_one, "unix");
    }
}
#endif


#if HAVE_AFAX25
static int ax25_info(void)
{
    FILE *f;
    char buffer[256], buf[16];
    char *src, *dst, *dev, *p;
    int st, vs, vr, sendq, recvq, ret;
    int new = -1;		/* flag for new (2.1.x) kernels */
    static char *ax25_state[5] =
    {
	N_("LISTENING"),
	N_("SABM SENT"),
	N_("DISC SENT"),
	N_("ESTABLISHED"),
	N_("RECOVERY")
    };
    if (!(f = proc_fopen(_PATH_PROCNET_AX25))) {
	if (errno != ENOENT) {
	    perror(_PATH_PROCNET_AX25);
	    return (-1);
	}
	if (flag_arg || flag_ver)
	    ESYSNOT("netstat", "AF AX25");
	if (flag_arg)
	    return (1);
	else
	    return (0);
    }
    printf(_("Active AX.25 sockets\n"));
    printf(_("Dest       Source     Device  State        Vr/Vs    Send-Q  Recv-Q\n"));
    while (fgets(buffer, 256, f)) {
	if (new == -1) {
	    if (!strncmp(buffer, "dest_addr", 9)) {
		new = 0;
		continue;	/* old kernels have a header line */
	    } else
		new = 1;
	}
	/*
	 * In a network connection with no user socket the Snd-Q, Rcv-Q
	 * and Inode fields are empty in 2.0.x and '*' in 2.1.x
	 */
	sendq = 0;
	recvq = 0;
	if (new == 0) {
	    dst = buffer;
	    src = buffer + 10;
	    dst[9] = 0;
	    src[9] = 0;
	    ret = sscanf(buffer + 20, "%s %d %d %d %*d %*d/%*d %*d/%*d %*d/%*d %*d/%*d %*d/%*d %*d %*d %*d %d %d %*d",
		   buf, &st, &vs, &vr, &sendq, &recvq);
	    if (ret != 4 && ret != 6) {
		printf(_("Problem reading data from %s\n"), _PATH_PROCNET_AX25);
		continue;
	    }
	    dev = buf;
	} else {
	    p = buffer;
	    while (*p != ' ') p++;
	    p++;
	    dev = p;
	    while (*p != ' ') p++;
	    *p++ = 0;
	    src = p;
	    while (*p != ' ') p++;
	    *p++ = 0;
	    dst = p;
	    while (*p != ' ') p++;
	    *p++ = 0;
	    ret = sscanf(p, "%d %d %d %*d %*d %*d %*d %*d %*d %*d %*d %*d %*d %*d %*d %*d %*d %d %d %*d",
		   &st, &vs, &vr, &sendq, &recvq);
	    if (ret != 3 && ret != 5) {
		    printf(_("problem reading data from %s\n"), _PATH_PROCNET_AX25);
		    continue;
	    }
	    /*
	     * FIXME: digipeaters should be handled somehow.
	     * For now we just strip them.
	     */
	    p = dst;
	    while (*p && *p != ',') p++;
	    *p = 0;
	}
	printf("%-9s  %-9s  %-6s  %-11s  %03d/%03d  %-6d  %-6d\n",
	       dst, src,
	       dev,
	       _(ax25_state[st]),
	       vr, vs, sendq, recvq);
    }
    fclose(f);
    return 0;
}
#endif


#if HAVE_AFIPX
static int ipx_info(void)
{
    FILE *f;
    char buf[256];
    unsigned long txq, rxq;
    unsigned int state;
    unsigned int uid;
    char *st;
    int nc;
    struct aftype *ap;
    struct passwd *pw;
    char sad[50], dad[50];
    struct sockaddr sa;
    unsigned sport = 0, dport = 0;
    struct stat s;
    
    f = proc_fopen(_PATH_PROCNET_IPX_SOCKET1);
    if (!f) {
        if (errno != ENOENT) {
            perror(_PATH_PROCNET_IPX_SOCKET1);
            return (-1);
        }
        f = proc_fopen(_PATH_PROCNET_IPX_SOCKET2);

        /* We need to check for directory */
        if (f) {
            if (fstat (fileno(f), &s) == -1 ||
                !S_ISREG(s.st_mode)) {
                fclose(f);
                f=NULL;
            }
        }

        if (!f) {
            if (errno != ENOENT) {
	        perror(_PATH_PROCNET_IPX_SOCKET2);
	        return (-1);
	    }
	    if (flag_arg || flag_ver)
	        ESYSNOT("netstat", "AF IPX");
	    if (flag_arg)
	        return (1);
 	    else
	        return (0);
        }
    }
    printf(_("Active IPX sockets\nProto Recv-Q Send-Q Local Address              Foreign Address            State"));	/* xxx */
    if (flag_exp > 1)
	printf(_(" User"));	/* xxx */
    printf("\n");
    if ((ap = get_afntype(AF_IPX)) == NULL) {
	EINTERN("netstat.c", "AF_IPX missing");
	fclose(f);
	return (-1);
    }
    if (fgets(buf, 255, f))
	/* eat line */;

    while (fgets(buf, 255, f) != NULL) {
	sscanf(buf, "%s %s %lX %lX %d %d",
	       sad, dad, &txq, &rxq, &state, &uid);
	if ((st = rindex(sad, ':'))) {
	    *st++ = '\0';
	    sscanf(st, "%X", &sport);	/* net byt order */
	    sport = ntohs(sport);
	} else {
	    EINTERN("netstat.c", "ipx socket format error in source port");
	    fclose(f);
	    return (-1);
	}
	nc = 0;
	if (strcmp(dad, "Not_Connected") != 0) {
	    if ((st = rindex(dad, ':'))) {
		*st++ = '\0';
		sscanf(st, "%X", &dport);	/* net byt order */
		dport = ntohs(dport);
	    } else {
		EINTERN("netstat.c", "ipx soket format error in destination port");
		fclose(f);
		return (-1);
	    }
	} else
	    nc = 1;

	switch (state) {
	case TCP_ESTABLISHED:
	    st = _("ESTAB");
	    break;

	case TCP_CLOSE:
	    st = "";
	    break;

	default:
	    st = _("UNK.");
	    break;
	}

	/* Fetch and resolve the Source */
	(void) ap->input(4, sad, &sa);
	safe_strncpy(buf, ap->sprint(&sa, flag_not & FLAG_NUM_HOST), sizeof(buf));
	snprintf(sad, sizeof(sad), "%s:%04X", buf, sport);

	if (!nc) {
	    /* Fetch and resolve the Destination */
	    (void) ap->input(4, dad, &sa);
	    safe_strncpy(buf, ap->sprint(&sa, flag_not & FLAG_NUM_HOST), sizeof(buf));
	    snprintf(dad, sizeof(dad), "%s:%04X", buf, dport);
	} else
	    strcpy(dad, "-");

	printf("IPX   %6ld %6ld %-26s %-26s %-5s", txq, rxq, sad, dad, st);
	if (flag_exp > 1) {
	    if (!(flag_not & FLAG_NUM_USER) && ((pw = getpwuid(uid)) != NULL))
		printf(" %-10s", pw->pw_name);
	    else
		printf(" %-10d", uid);
	}
	printf("\n");
    }
    fclose(f);
    return 0;
}
#endif

#if HAVE_AFBLUETOOTH
const char *bluetooth_state(int state)
{
    switch (state) {
	case BT_CONNECTED:
	    return _("CONNECTED");
	case BT_OPEN:
	    return _("OPEN");
	case BT_BOUND:
	    return _("BOUND");
	case BT_LISTEN:
	    return _("LISTEN");
	case BT_CONNECT:
	    return _("CONNECT");
	case BT_CONNECT2:
	    return _("CONNECT2");
	case BT_CONFIG:
	    return _("CONFIG");
	case BT_DISCONN:
	    return _("DISCONN");
	case BT_CLOSED:
	    return _("CLOSED");
	default:
	    return _("UNKNOWN");
    }
}

static void l2cap_do_one(int nr, const char *line, const char *prot)
{
    char daddr[18], saddr[18];
    unsigned state, psm, dcid, scid, imtu, omtu, sec_level;
    int num;
    const char *bt_state, *bt_sec_level;

    num = sscanf(line, "%17s %17s %d %d 0x%04x 0x%04x %d %d %d",
	daddr, saddr, &state, &psm, &dcid, &scid, &imtu, &omtu, &sec_level);

    if (num < 9) {
	fprintf(stderr, _("warning, got bogus l2cap line.\n"));
	return;
    }

    if (flag_lst && !(state == BT_LISTEN || state == BT_BOUND))
	return;
    if (!(flag_all || flag_lst) && (state == BT_LISTEN || state == BT_BOUND))
	return;

    bt_state = bluetooth_state(state);
    switch (sec_level) {
	case BT_SECURITY_SDP:
	    bt_sec_level = _("SDP");
	    break;
	case BT_SECURITY_LOW:
	    bt_sec_level = _("LOW");
	    break;
	case BT_SECURITY_MEDIUM:
	    bt_sec_level = _("MEDIUM");
	    break;
	case BT_SECURITY_HIGH:
	    bt_sec_level = _("HIGH");
	    break;
	default:
	    bt_sec_level = _("UNKNOWN");
    }

    printf("l2cap  %-17s %-17s %-9s %7d 0x%04x 0x%04x %7d %7d %-7s\n",
	(strcmp (daddr, "00:00:00:00:00:00") == 0 ? "*" : daddr),
	(strcmp (saddr, "00:00:00:00:00:00") == 0 ? "*" : saddr),
	bt_state, psm, dcid, scid, imtu, omtu, bt_sec_level);
}

static int l2cap_info(void)
{
    printf("%-6s %-17s %-17s %-9s %7s %-6s %-6s %7s %7s %-7s\n",
	"Proto", "Destination", "Source", "State", "PSM", "DCID", "SCID", "IMTU", "OMTU", "Security");
    INFO_GUTS(_PATH_SYS_BLUETOOTH_L2CAP, "AF BLUETOOTH", l2cap_do_one, "l2cap");
}

static void rfcomm_do_one(int nr, const char *line, const char *prot)
{
    char daddr[18], saddr[18];
    unsigned state, channel;
    int num;
    const char *bt_state;

    num = sscanf(line, "%17s %17s %d %d", daddr, saddr, &state, &channel);
    if (num < 4) {
	fprintf(stderr, _("warning, got bogus rfcomm line.\n"));
	return;
    }

    if (flag_lst && !(state == BT_LISTEN || state == BT_BOUND))
	return;
    if (!(flag_all || flag_lst) && (state == BT_LISTEN || state == BT_BOUND))
	return;

    bt_state = bluetooth_state(state);
    printf("rfcomm %-17s %-17s %-9s %7d\n",
	(strcmp (daddr, "00:00:00:00:00:00") == 0 ? "*" : daddr),
	(strcmp (saddr, "00:00:00:00:00:00") == 0 ? "*" : saddr),
	bt_state, channel);
}

static int rfcomm_info(void)
{
    printf("%-6s %-17s %-17s %-9s %7s\n", "Proto", "Destination", "Source", "State", "Channel");
    INFO_GUTS(_PATH_SYS_BLUETOOTH_RFCOMM, "AF BLUETOOTH", rfcomm_do_one, "rfcomm");
}
#endif

static int iface_info(void)
{
    if (skfd < 0) {
	if ((skfd = sockets_open(0)) < 0) {
	    perror("socket");
	    exit(1);
	}
	printf(_("Kernel Interface table\n"));
    }
    if (flag_exp < 2) {
	ife_short = 1;
	printf(_("Iface      MTU    RX-OK RX-ERR RX-DRP RX-OVR    TX-OK TX-ERR TX-DRP TX-OVR Flg\n"));
    }

    if (for_all_interfaces(do_if_print, &flag_all) < 0) {
	perror(_("missing interface information"));
	exit(1);
    }
    if (flag_cnt)
	if_cache_free();
    else {
	close(skfd);
	skfd = -1;
    }

    return 0;
}


static void version(void)
{
    printf("%s\n%s\n%s\n", Release, Signature, Features);
    exit(E_VERSION);
}


static void usage(void)
{
    fprintf(stderr, _("usage: netstat [-vWeenNcCF] [<Af>] -r         netstat {-V|--version|-h|--help}\n"));
    fprintf(stderr, _("       netstat [-vWnNcaeol] [<Socket> ...]\n"));
    fprintf(stderr, _("       netstat { [-vWeenNac] -i | [-cnNe] -M | -s [-6tuw] }\n\n"));

    fprintf(stderr, _("        -r, --route              display routing table\n"));
    fprintf(stderr, _("        -i, --interfaces         display interface table\n"));
    fprintf(stderr, _("        -g, --groups             display multicast group memberships\n"));
    fprintf(stderr, _("        -s, --statistics         display networking statistics (like SNMP)\n"));
#if HAVE_FW_MASQUERADE
    fprintf(stderr, _("        -M, --masquerade         display masqueraded connections\n\n"));
#endif

    fprintf(stderr, _("        -v, --verbose            be verbose\n"));
    fprintf(stderr, _("        -W, --wide               don't truncate IP addresses\n"));
    fprintf(stderr, _("        -n, --numeric            don't resolve names\n"));
    fprintf(stderr, _("        --numeric-hosts          don't resolve host names\n"));
    fprintf(stderr, _("        --numeric-ports          don't resolve port names\n"));
    fprintf(stderr, _("        --numeric-users          don't resolve user names\n"));
    fprintf(stderr, _("        -N, --symbolic           resolve hardware names\n"));
    fprintf(stderr, _("        -e, --extend             display other/more information\n"));
    fprintf(stderr, _("        -p, --programs           display PID/Program name for sockets\n"));
    fprintf(stderr, _("        -o, --timers             display timers\n"));
    fprintf(stderr, _("        -c, --continuous         continuous listing\n\n"));
    fprintf(stderr, _("        -l, --listening          display listening server sockets\n"));
    fprintf(stderr, _("        -a, --all                display all sockets (default: connected)\n"));
    fprintf(stderr, _("        -F, --fib                display Forwarding Information Base (default)\n"));
    fprintf(stderr, _("        -C, --cache              display routing cache instead of FIB\n"));
#if HAVE_SELINUX
    fprintf(stderr, _("        -Z, --context            display SELinux security context for sockets\n"));
#endif

    fprintf(stderr, _("\n  <Socket>={-t|--tcp} {-u|--udp} {-U|--udplite} {-w|--raw} {-x|--unix}\n"));
    fprintf(stderr, _("           --ax25 --ipx --netrom\n"));
    fprintf(stderr, _("  <AF>=Use '-6|-4' or '-A <af>' or '--<af>'; default: %s\n"), DFLT_AF);
    fprintf(stderr, _("  List of possible address families (which support routing):\n"));
    print_aflist(1); /* 1 = routeable */
    exit(E_USAGE);
}


int main
 (int argc, char *argv[]) {
    int i;
    int lop;
    static struct option longopts[] =
    {
	AFTRANS_OPTS,
	{"version", 0, 0, 'V'},
	{"interfaces", 0, 0, 'i'},
	{"help", 0, 0, 'h'},
	{"route", 0, 0, 'r'},
#if HAVE_FW_MASQUERADE
	{"masquerade", 0, 0, 'M'},
#endif
	{"protocol", 1, 0, 'A'},
	{"tcp", 0, 0, 't'},
	{"sctp", 0, 0, 'S'},
	{"udp", 0, 0, 'u'},
        {"udplite", 0, 0, 'U'},
	{"raw", 0, 0, 'w'},
	{"unix", 0, 0, 'x'},
	{"l2cap", 0, 0, '2'},
	{"rfcomm", 0, 0, 'f'},
	{"listening", 0, 0, 'l'},
	{"all", 0, 0, 'a'},
	{"timers", 0, 0, 'o'},
	{"continuous", 0, 0, 'c'},
	{"extend", 0, 0, 'e'},
	{"programs", 0, 0, 'p'},
	{"verbose", 0, 0, 'v'},
	{"statistics", 0, 0, 's'},
	{"wide", 0, 0, 'W'},
	{"numeric", 0, 0, 'n'},
	{"numeric-hosts", 0, 0, '!'},
	{"numeric-ports", 0, 0, '@'},
	{"numeric-users", 0, 0, '#'},
	{"symbolic", 0, 0, 'N'},
	{"cache", 0, 0, 'C'},
	{"fib", 0, 0, 'F'},
	{"groups", 0, 0, 'g'},
	{"context", 0, 0, 'Z'},
	{NULL, 0, 0, 0}
    };

#if I18N
    setlocale (LC_ALL, "");
    bindtextdomain("net-tools", "/usr/share/locale");
    textdomain("net-tools");
#endif
    getroute_init();		/* Set up AF routing support */

    afname[0] = '\0';
    while ((i = getopt_long(argc, argv, "A:CFMacdeghilnNoprsStuUvVWwx64?Z", longopts, &lop)) != EOF)
	switch (i) {
	case -1:
	    break;
	case 1:
	    if (lop < 0 || lop >= AFTRANS_CNT) {
		EINTERN("netstat.c", "longopts 1 range");
		break;
	    }
	    if (aftrans_opt(longopts[lop].name))
		exit(1);
	    break;
	case 'A':
	    if (aftrans_opt(optarg))
		exit(1);
	    break;
	case 'M':
	    flag_mas++;
	    break;
	case 'a':
	    flag_all++;
	    break;
	case 'l':
	    flag_lst++;
	    break;
	case 'c':
	    flag_cnt++;
	    break;

	case 'd':
	    flag_deb++;
	    break;
	case 'g':
	    flag_igmp++;
	    break;
	case 'e':
	    flag_exp++;
	    break;
	case 'p':
	    flag_prg++;
	    break;
	case 'i':
	    flag_int++;
	    break;
	case 'W':
	    flag_wide++;
	    break;
	case 'n':
	    flag_not |= FLAG_NUM;
	    break;
	case '!':
	    flag_not |= FLAG_NUM_HOST;
	    break;
	case '@':
	    flag_not |= FLAG_NUM_PORT;
	    break;
	case '#':
	    flag_not |= FLAG_NUM_USER;
	    break;
	case 'N':
	    flag_not |= FLAG_SYM;
	    break;
	case 'C':
	    flag_cf |= FLAG_CACHE;
	    break;
	case 'F':
	    flag_cf |= FLAG_FIB;
	    break;
	case 'o':
	    flag_opt++;
	    break;
	case '6':
	    if (aftrans_opt("inet6"))
		exit(1);
	    break;
	case '4':
	    if (aftrans_opt("inet"))
		exit(1);
	    break;
	case 'V':
	    version();
	    /*NOTREACHED */
	case 'v':
	    flag_ver |= FLAG_VERBOSE;
	    break;
	case 'r':
	    flag_rou++;
	    break;
	case 't':
	    flag_tcp++;
	    break;
	case 'S':
	    flag_sctp++;
	    break;
	case 'u':
	    flag_udp++;
	    break;
        case 'U':
	    flag_udplite++;
	    break;
	case 'w':
	    flag_raw++;
	    break;
        case '2':
	    flag_l2cap++;
	    break;
        case 'f':
	    flag_rfcomm++;
	    break;
	case 'x':
	    if (aftrans_opt("unix"))
		exit(1);
	    break;
	case 'Z':
#if HAVE_SELINUX
	    if (is_selinux_enabled() <= 0) {
		fprintf(stderr, _("SELinux is not enabled on this machine.\n"));
		exit(1);
	    }
	    flag_prg++;
	    flag_selinux++;
#else
            fprintf(stderr, _("SELinux is not enabled for this application.\n"));
	    exit(1);
#endif

	    break;
	case '?':
	case 'h':
	    usage();
	case 's':
	    flag_sta++;
	}

    if (flag_int + flag_rou + flag_mas + flag_sta > 1)
	usage();

    if ((flag_inet || flag_inet6 || flag_sta) && 
        !(flag_tcp || flag_sctp || flag_udp || flag_udplite || flag_raw))
	   flag_tcp = flag_sctp = flag_udp = flag_udplite = flag_raw = 1;

    if ((flag_tcp || flag_sctp || flag_udp || flag_udplite || flag_raw || flag_igmp) && 
        !(flag_inet || flag_inet6))
        flag_inet = flag_inet6 = 1;

    if (flag_bluetooth && !(flag_l2cap || flag_rfcomm))
	   flag_l2cap = flag_rfcomm = 1;

    flag_arg = flag_tcp + flag_sctp + flag_udplite + flag_udp + flag_raw + flag_unx 
        + flag_ipx + flag_ax25 + flag_netrom + flag_igmp + flag_x25 + flag_rose
	+ flag_l2cap + flag_rfcomm;

    if (flag_mas) {
#if HAVE_FW_MASQUERADE && HAVE_AFINET
#if MORE_THAN_ONE_MASQ_AF
	if (!afname[0])
	    strcpy(afname, DFLT_AF);
#endif
	for (;;) {
	    i = ip_masq_info(flag_not & FLAG_NUM_HOST,
			     flag_not & FLAG_NUM_PORT, flag_exp);
	    if (i || !flag_cnt)
		break;
	    wait_continous();
	}
#else
	ENOSUPP("netstat", "FW_MASQUERADE");
	i = -1;
#endif
	return (i);
    }

    if (flag_sta) {
        if (!afname[0])
            strcpy(afname, DFLT_AF);
            
        if (!strcmp(afname, "inet")) {
#if HAVE_AFINET
            inittab();
            parsesnmp(flag_raw, flag_tcp, flag_udp, flag_sctp);
#else
            ENOSUPP("netstat", "AF INET");
#endif
        } else if(!strcmp(afname, "inet6")) {
#if HAVE_AFINET6
            inittab6();
            parsesnmp6(flag_raw, flag_tcp, flag_udp);
#else
            ENOSUPP("netstat", "AF INET6");
#endif
        } else {
          printf(_("netstat: No statistics support for specified address family: %s\n"), afname);
          exit(1);
        }
        exit(0);
    }
    
    if (flag_rou) {
	int options = 0;

	if (!afname[0])
	    strcpy(afname, DFLT_AF);

	if (flag_exp == 2)
	    flag_exp = 1;
	else if (flag_exp == 1)
	    flag_exp = 2;

	options = (flag_exp & FLAG_EXT) | flag_not | flag_cf | flag_ver;
	if (!flag_cf)
	    options |= FLAG_FIB;

	for (;;) {
	    i = route_info(afname, options);
	    if (i || !flag_cnt)
		break;
            wait_continous();
	}
	return (i);
    }
    if (flag_int) {
	for (;;) {
	    i = iface_info();
	    if (!flag_cnt || i)
		break;
            wait_continous();
	}
	return (i);
    }
    for (;;) {
	if (!flag_arg || flag_tcp || flag_sctp || flag_udp || flag_udplite || flag_raw) {
#if HAVE_AFINET
	    prg_cache_load();
	    printf(_("Active Internet connections "));	/* xxx */

	    if (flag_all)
		printf(_("(servers and established)"));
	    else {
	      if (flag_lst)
		printf(_("(only servers)"));
	      else
		printf(_("(w/o servers)"));
	    }
	    printf(_("\nProto Recv-Q Send-Q Local Address           Foreign Address         State      "));	/* xxx */
	    if (flag_exp > 1)
		printf(_(" User       Inode     "));
	    print_progname_banner();
	    print_selinux_banner();
	    if (flag_opt)
		printf(_(" Timer"));	/* xxx */
	    printf("\n");
#else
	    if (flag_arg) {
		i = 1;
		ENOSUPP("netstat", "AF INET");
	    }
#endif
	}
#if HAVE_AFINET
	if (!flag_arg || flag_tcp) {
	    i = tcp_info();
	    if (i)
		return (i);
	}

	if (!flag_arg || flag_sctp) {
	    i = sctp_info();
	    if (i)
		return (i);
	}

	if (!flag_arg || flag_udp) {
	    i = udp_info();
	    if (i)
		return (i);
	}

	if (!flag_arg || flag_udplite) {
	    i = udplite_info();
	    if (i)
		return (i);
	}

	if (!flag_arg || flag_raw) {
	    i = raw_info();
	    if (i)
		return (i);
	}

	if (flag_igmp) {
#if HAVE_AFINET6
	    printf( "IPv6/");
#endif
	    printf( _("IPv4 Group Memberships\n") );
	    printf( _("Interface       RefCnt Group\n") );
	    printf( "--------------- ------ ---------------------\n" );
	    i = igmp_info();
	    if (i)
	        return (i);
	}
#endif

	if (!flag_arg || flag_unx) {
#if HAVE_AFUNIX
	    prg_cache_load();
	    i = unix_info();
	    if (i)
		return (i);
#else
	    if (flag_arg) {
		i = 1;
		ENOSUPP("netstat", "AF UNIX");
	    }
#endif
	}
	if (!flag_arg || flag_ipx) {
#if HAVE_AFIPX
	    i = ipx_info();
	    if (i)
		return (i);
#else
	    if (flag_arg) {
		i = 1;
		ENOSUPP("netstat", "AF IPX");
	    }
#endif
	}
	if (!flag_arg || flag_ax25) {
#if HAVE_AFAX25
	    i = ax25_info();
	    if (i)
		return (i);
#else
	    if (flag_arg) {
		i = 1;
		ENOSUPP("netstat", "AF AX25");
	    }
#endif
	}
	if(!flag_arg || flag_x25) {
#if HAVE_AFX25
	    /* FIXME */
	    i = x25_info();
	    if (i)
		return(i);
#else
	    if (flag_arg) {
		i = 1;
		ENOSUPP("netstat", "AF X25");
	    }
#endif
	}
	if (!flag_arg || flag_netrom) {
#if HAVE_AFNETROM
	    i = netrom_info();
	    if (i)
		return (i);
#else
	    if (flag_arg) {
		i = 1;
		ENOSUPP("netstat", "AF NETROM");
	    }
#endif
	}
	if (!flag_arg || flag_rose) {
#if 0 && HAVE_AFROSE
          i = rose_info();
          if (i)
            return (i);
#else
          if (flag_arg) {
            i = 1;
            ENOSUPP("netstat", "AF ROSE");
          }
#endif
        }

	if (!flag_arg || flag_l2cap || flag_rfcomm) {
#if HAVE_AFBLUETOOTH
	    printf(_("Active Bluetooth connections "));	/* xxx */

	    if (flag_all)
		printf(_("(servers and established)"));
	    else {
	      if (flag_lst)
		printf(_("(only servers)"));
	      else
		printf(_("(w/o servers)"));
	    }
	    printf("\n");
#else
	    if (flag_arg) {
		i = 1;
		ENOSUPP("netstat", "AF BLUETOOTH");
	    }
#endif
	}
#if HAVE_AFBLUETOOTH
	if (!flag_arg || flag_l2cap) {
	    i = l2cap_info();
	    if (i)
		return (i);
	}
	if (!flag_arg || flag_rfcomm) {
	    i = rfcomm_info();
	    if (i)
		return (i);
	}
#endif

	if (!flag_cnt || i)
	    break;
        wait_continous();
	prg_cache_clear();
    }
    return (i);
}
