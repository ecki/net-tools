/*
 * lib/net-locale.h	Headerfile for the NLS functions
 *
 * NET-LIB	
 *
 * Version:	lib/net-locale.h 0.51 (1996-03-22)
 *
 * Author:	Bernd Eckenfels <net-tools@lina.inka.de>
 *		Copyright 1995-1996 Bernd Eckebnfels, Germany
 *
 * Modifications:
 *960125 {0.50}	Bernd Eckenfels:	included Header, reformated
 *960322 {0.51}	Bernd Eckenfels:	moved into lib/
 *
 */
#ifndef NET_LOCALE_H
#define NET_LOCALE_H

#if NLS
#  include <locale.h>
#  include <nl_types.h>

#  ifndef EXTERN
#    define EXTERN
#  else
#    undef EXTERN
#    define EXTERN extern
#  endif

   EXTERN nl_catd catfd;

   char *strsave (char *);
   char *str_in_buff (char *, int, char *);

#  define NLS_CATINIT catinit ();
#  define NLS_CATCLOSE(catfd) catclose (catfd);
#  define NLS_CATGETS(catfd, arg1, arg2, fmt) \
	catgets ((catfd), (arg1), (arg2), (fmt))
#  define NLS_CATSAVE(catfd, arg1, arg2, fmt) \
	strsave (catgets ((catfd), (arg1), (arg2), (fmt)))
#  define NLS_CATBUFF(catfd, arg1, arg2, fmt, buf, len) \
	str_in_buff (buf, len, catgets ((catfd), (arg1), (arg2), (fmt)))
#  include "nettools-nls.h"
#else
#  define NLS_CATINIT
#  define NLS_CATCLOSE(catfd)
#  define NLS_CATGETS(catfd, arg1, arg2, fmt) fmt
#  define NLS_CATSAVE(catfd, arg1, arg2, fmt) fmt
#  define NLS_CATBUFF(catfd, arg1, arg2, fmt, buf, len) strcpy (buf, fmt)
#endif

#endif /* NET_LOCALE_H */
