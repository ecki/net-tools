#
# lib/Makefile	Makefile for the net-lib function collection
#
# NET-LIB	A collection of functions used from the base set of the
#		NET-2 Networking Distribution for the LINUX operating
#		system.  (net-tools, net-drivers)
#
# Version:	lib/Makefile 1.27 (1996-04-13)
#
# Author:	Fred N. van Kempen, <waltje@uwalt.nl.mugnet.org>
#		Copyright 1993 MicroWalt Corporation
#
# Modification:
#960125	{1.21}	Bernd Eckenfels:	reformated, layout
#960203 {1.22}	Bernd Eckenfels:	afrt.o will include all the 
#					routing support into the binaries.
#960206 {1.23}	Bernd Eckenfels:	better solution wit route_init
#960215 {1.24}	Bernd Eckenfels:	ax25_rt.o added
#960221 {1.25}	Bernd Eckenfels:	getroute/setroute.o
#960322 {1.26}	Bernd Eckenfels:	ACTOBJS support
#960413 {1.27}	Bernd Eckenfels:	Mike McLagan FRAD Support (frame.c)
#960809 {1.xx}	Frank Strauss:		inet6.c inet6_gr.c
#
#		This program is free software; you can redistribute it
#		and/or  modify it under  the terms of  the GNU General
#		Public  License as  published  by  the  Free  Software
#		Foundation;  either  version 2 of the License, or  (at
#		your option) any later version.
#


HWOBJS	 = hw.o loopback.o slip.o ether.o ax25.o ppp.o arcnet.o tr.o tunnel.o frame.o sit.o rose.o ash.o fddi.o
AFOBJS	 = unix.o inet.o inet6.o ax25.o ipx.o ddp.o ipx.o netrom.o af.o rose.o econet.o
AFGROBJS = inet_gr.o inet6_gr.o ipx_gr.o ddp_gr.o netrom_gr.o ax25_gr.o rose_gr.o getroute.o
AFSROBJS = inet_sr.o inet6_sr.o netrom_sr.o ipx_sr.o setroute.o
ACTOBJS  = slip_ac.o ppp_ac.o activate.o
VARIA	 = getargs.o masq_info.o
NLSMISC  = net-string.o

OBJS	= $(NLSMISC) $(VARIA) $(AFOBJS) $(HWOBJS) \
		$(AFGROBJS) $(AFSROBJS) $(ACTOBJS)


# This can be overwritten by the TOPLEVEL Makefile
TOPDIR=..
COPTS = -O2 -Wall -fomit-frame-pointer #-DDEBUG
LOPTS = -s

CFLAGS  = $(COPTS) -I. -I$(TOPDIR) -I$(TOPDIR)/include
LDFLAGS = $(LOPTS)

all:	libsupport.a

libsupport.a:	Makefile $(TOPDIR)/config.h $(OBJS)
			@echo Building libsupport.a
			@rm -f libsupport.a
			@ar rcs libsupport.a $(OBJS)
			@ranlib libsupport.a

clean:
		rm -f *.o *~ *.orig

clobber:	clean
		rm -f *.a

# End of lib/Makefile.
