/*
 * lib/support.h      This file contains the definitions of what is in the
 *                      support library.  Most of all, it defines structures
 *                      for accessing support modules, and the function proto-
 *                      types.
 *
 * NET-LIB      A collection of functions used from the base set of the
 *              NET-3 Networking Distribution for the LINUX operating
 *              system. (net-tools, net-drivers)
 *
 * Version:     lib/net-support.h 1.34 (1996-04-13)
 *
 * Maintainer:  Bernd 'eckes' Eckenfels, <net-tools@lina.inka.de>
 *
 * Author:      Fred N. van Kempen, <waltje@uwalt.nl.mugnet.org>
 *              Copyright 1993 MicroWalt Corporation
 *
 * Modifications:
 *960125 {1.20} Bernd Eckenfels:        reformated, layout
 *960202 {1.30} Bernd Eckenfels:        rprint in aftype
 *960206 {1.31} Bernd Eckenfels:        route_init
 *960219 {1.32} Bernd Eckenfels:        type for ap->input()
 *960322 {1.33} Bernd Eckenfels:        activate_ld and const in get_hwtype
 *960413 {1.34} Bernd Eckenfels:        new RTACTION suport
 *
 *              This program is free software; you can redistribute it
 *              and/or  modify it under  the terms of  the GNU General
 *              Public  License as  published  by  the  Free  Software
 *              Foundation;  either  version 2 of the License, or  (at
 *              your option) any later version.
 */
#include <sys/socket.h>

/* This structure defines protocol families and their handlers. */
struct aftype {
    char *name;
    char *title;
    int af;
    int alen;
    char *(*print) (unsigned char *);
    char *(*sprint) (struct sockaddr *, int numeric);
    int (*input) (int type, char *bufp, struct sockaddr *);
    void (*herror) (char *text);
    int (*rprint) (int options);
    int (*rinput) (int typ, int ext, char **argv);

    /* may modify src */
    int (*getmask) (char *src, struct sockaddr * mask, char *name);

    int fd;
    char *flag_file;
};

extern struct aftype *aftypes[];

/* This structure defines hardware protocols and their handlers. */
struct hwtype {
    char *name;
    char *title;
    int type;
    int alen;
    char *(*print) (unsigned char *);
    char *(*sprint) (struct sockaddr *);
    int (*input) (char *, struct sockaddr *);
    int (*activate) (int fd);
};


extern struct hwtype *get_hwtype(const char *name);
extern struct hwtype *get_hwntype(int type);
extern struct aftype *get_aftype(const char *name);
extern struct aftype *get_afntype(int type);

extern int getargs(char *string, char *arguments[]);

extern int get_socket_for_af(int af);

extern void getroute_init(void);
extern void setroute_init(void);
extern void activate_init(void);
extern int route_info(const char *afname, int flags);
extern int route_edit(int action, const char *afname, int flags, char **argv);
extern int activate_ld(const char *hwname, int fd);

#define RTACTION_ADD   1
#define RTACTION_DEL   2
#define RTACTION_HELP  3
#define RTACTION_FLUSH 4
#define RTACTION_SHOW  5

#define FLAG_EXT      3		/* AND-Mask */
#define FLAG_NUM      4
#define FLAG_SYM      8
#define FLAG_CACHE   16
#define FLAG_FIB     32
#define FLAG_VERBOSE 64

extern int ip_masq_info(int numeric, int ext);

extern int INET_rprint(int options);
extern int INET6_rprint(int options);
extern int DDP_rprint(int options);
extern int IPX_rprint(int options);
extern int NETROM_rprint(int options);
extern int AX25_rprint(int options);

extern int INET_rinput(int action, int flags, char **argv);
extern int INET6_rinput(int action, int flags, char **argv);
extern int DDP_rinput(int action, int flags, char **argv);
extern int IPX_rinput(int action, int flags, char **argv);
extern int NETROM_rinput(int action, int flags, char **argv);
extern int AX25_rinput(int action, int flags, char **argv);

extern int aftrans_opt(const char *arg);
extern void aftrans_def(char *tool, char *argv0, char *dflt);

extern char *get_sname(int socknumber, char *proto, int numeric);

extern char *safe_strncpy(char *dst, const char *src, size_t size);

extern int flag_unx;
extern int flag_ipx;
extern int flag_ax25;
extern int flag_ddp;
extern int flag_netrom;
extern int flag_inet;
extern int flag_inet6;

extern char afname[];

#define AFTRANS_OPTS \
	{"ax25",	0,	0,	1}, \
	{"ip",		0,	0,	1}, \
	{"ipx",         0,	0,	1}, \
	{"appletalk",	0,	0,	1}, \
	{"netrom",	0,	0,	1}, \
	{"inet",	0,	0,	1}, \
	{"inet6",	0,	0,	1}, \
	{"ddp",		0,	0,	1}, \
	{"unix",	0,	0,	1}, \
	{"tcpip",	0,	0,	1}
#define AFTRANS_CNT 10

#define EINTERN(file, text) fprintf(stderr, \
	"%s: Internal Error `%s'.\n",file,text);

#define ENOSUPP(A,B)	fprintf(stderr,\
                                _("%s: feature `%s' not supported.\n" \
				  "Please recompile `net-tools' with "\
				  "newer kernel source or full configuration.\n"),A,B)

#define ESYSNOT(A,B)	fprintf(stderr, _("%s: no support for `%s' on this system.\n"),A,B)

#define E_NOTFOUND	8
#define E_SOCK		7
#define E_LOOKUP	6
#define E_VERSION	5
#define E_USAGE		4
#define E_OPTERR	3
#define E_INTERN	2
#define E_NOSUPP	1

/* End of lib/support.h */
