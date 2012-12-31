#
# Makefile	Main Makefile for the net-tools Package
#
# NET-TOOLS	A collection of programs that form the base set of the
#		NET-3 Networking Distribution for the LINUX operating
#		system.
#
# Author:	Bernd Eckenfels <net-tools@lina.inka.de>
#		Copyright 1995-1996 Bernd Eckenfels, Germany
#
# URLs:		http://net-tools.sourceforge.net/
#
# Based on:	Fred N. van Kempen, <waltje@uwalt.nl.mugnet.org>
#		Copyright 1988-1993 MicroWalt Corporation
#
# Modifications:
#		Extensively modified from 01/21/94 onwards by
#		Alan Cox <A.Cox@swansea.ac.uk>
#		Copyright 1993-1994 Swansea University Computer Society
#
# Be careful! 
# This Makefile doesn't describe complete dependencies for all include files.
# If you change include files you might need to do make clean. 
#

# set the base of the Installation 
# BASEDIR = /mnt
BASEDIR ?= $(DESTDIR)

# path to the net-lib support library. Default: lib
NET_LIB_PATH = lib
NET_LIB_NAME = net-tools

PROGS	:= ifconfig hostname arp netstat route rarp slattach plipconfig nameif

-include config.make
ifeq ($(HAVE_IP_TOOLS),1)
PROGS   += iptunnel ipmaddr
endif
ifeq ($(HAVE_MII),1)
PROGS	+= mii-tool
endif

# Compiler and Linker Options
# You may need to uncomment and edit these if you are using libc5 and IPv6.
CFLAGS ?= -O2 -g
CFLAGS += -Wall
CFLAGS += -fno-strict-aliasing # code needs a lot of work before strict aliasing is safe
CPPFLAGS += -D_GNU_SOURCE
RESLIB = # -L/usr/inet6/lib -linet6

ifeq ($(HAVE_AFDECnet),1)
DNLIB = -ldnet
endif

# -------- end of user definitions --------

MAINTAINER = net-tools-devel@lists.sourceforge.net
RELEASE	   = 1.70-alpha

.EXPORT_ALL_VARIABLES:

ifeq ("$(NET_LIB_PATH)","lib2")
TOPDIR   = ..
else
TOPDIR  := $(shell if [ "$$PWD" != "" ]; then echo $$PWD; else pwd; fi)
endif

NET_LIB = $(NET_LIB_PATH)/lib$(NET_LIB_NAME).a

ifeq ($(HAVE_SELINUX),1)
LDFLAGS += -lselinux
CFLAGS += -DHAVE_SELINUX
endif

CPPFLAGS += -I. -I$(TOPDIR)/include -I$(NET_LIB_PATH)
LDFLAGS  += -L$(NET_LIB_PATH)

SUBDIRS	= man/ $(NET_LIB_PATH)/

ifeq ($(origin CC), undefined)
CC	= gcc
endif
LD	= $(CC)

NLIB	= -l$(NET_LIB_NAME)

%.o:		%.c config.h version.h intl.h lib/net-features.h $<
		$(CC) $(CFLAGS) $(CPPFLAGS) -c $<

all:		config.h version.h subdirs $(PROGS)

config: 	cleanconfig config.h

install:	all savebin installbin installdata

update: 	all installbin installdata

mostlyclean:
		rm -f *.o DEADJOE config.new *~ *.orig lib/*.o

clean: mostlyclean
		rm -f $(PROGS)
		@for i in $(SUBDIRS); do (cd $$i && $(MAKE) clean) ; done
		@cd po && $(MAKE) clean

cleanconfig:
		rm -f config.h

clobber: 	clean
		rm -f $(PROGS) config.h version.h config.status config.make
		@for i in $(SUBDIRS); do (cd $$i && $(MAKE) clobber) ; done


dist:
		rm -rf net-tools-$(RELEASE)
		git archive --prefix=net-tools-$(RELEASE)/ HEAD | tar xf -
		$(MAKE) -C net-tools-$(RELEASE)/po $@
		tar cf - net-tools-$(RELEASE)/ | xz > net-tools-$(RELEASE).tar.xz
		rm -rf net-tools-$(RELEASE)

distcheck:	dist
		tar xf net-tools-$(RELEASE).tar.xz
		yes "" | $(MAKE) -C net-tools-$(RELEASE) config
		$(MAKE) -C net-tools-$(RELEASE)
		rm -rf net-tools-$(RELEASE)
		@printf "\nThe tarball is ready to go:\n%s\n" "`du -b net-tools-$(RELEASE).tar.xz`"

config.h: 	config.in Makefile 
		@echo "Configuring the Linux net-tools (NET-3 Base Utilities)..." ; echo
		@if [ config.status -nt config.in ]; \
			then ./configure.sh config.status; \
		   else ./configure.sh config.in; \
		 fi


version.h:	Makefile
		@echo "#define RELEASE \"net-tools $(RELEASE)\"" >version.h


$(NET_LIB):	config.h version.h intl.h libdir

i18n.h:		i18ndir

libdir:		version.h
		@$(MAKE) -C $(NET_LIB_PATH)

i18ndir:
		@$(MAKE) -C po

# use libdir target for lib/ to avoid parallel build issues
subdirs:	libdir
		@for i in $(SUBDIRS:$(NET_LIB_PATH)/=); do $(MAKE) -C $$i || exit $$? ; done

ifconfig:	$(NET_LIB) ifconfig.o
		$(CC) $(LDFLAGS) -o ifconfig ifconfig.o $(NLIB) $(RESLIB)
		
nameif:	nameif.o
		$(CC) $(LDFLAGS) -o nameif nameif.o 

hostname:	hostname.o
		$(CC) $(LDFLAGS) -o hostname hostname.o $(DNLIB)

route:		$(NET_LIB) route.o
		$(CC) $(LDFLAGS) -o route route.o $(NLIB) $(RESLIB)

arp:		$(NET_LIB) arp.o
		$(CC) $(LDFLAGS) -o arp arp.o $(NLIB) $(RESLIB)

rarp:		$(NET_LIB) rarp.o
		$(CC) $(LDFLAGS) -o rarp rarp.o $(NLIB)

slattach:	$(NET_LIB) slattach.o
		$(CC) $(LDFLAGS) -o slattach slattach.o $(NLIB)

plipconfig:	$(NET_LIB) plipconfig.o
		$(CC) $(LDFLAGS) -o plipconfig plipconfig.o $(NLIB)

netstat:	$(NET_LIB) netstat.o statistics.o
		$(CC) $(LDFLAGS) -o netstat netstat.o statistics.o $(NLIB) $(RESLIB)

iptunnel:	$(NET_LIB) iptunnel.o
		$(CC) $(LDFLAGS) -o iptunnel iptunnel.o $(NLIB) $(RESLIB)

ipmaddr:	$(NET_LIB) ipmaddr.o
		$(CC) $(LDFLAGS) -o ipmaddr ipmaddr.o $(NLIB) $(RESLIB)

mii-tool:	mii-tool.o
		$(CC) $(LDFLAGS) -o mii-tool mii-tool.o

installbin:
	@echo
	@echo "######################################################"
	@echo "Notice: ifconfig and route are now installed into /bin"
	@echo "######################################################"
	@echo
	install -m 0755 -d ${BASEDIR}/sbin
	install -m 0755 -d ${BASEDIR}/bin
	install -m 0755 arp        ${BASEDIR}/sbin
	install -m 0755 hostname   ${BASEDIR}/bin
	install -m 0755 ifconfig   ${BASEDIR}/bin
	install -m 0755 nameif     ${BASEDIR}/sbin
	install -m 0755 netstat    ${BASEDIR}/bin
	install -m 0755 plipconfig $(BASEDIR)/sbin
	install -m 0755 rarp       ${BASEDIR}/sbin
	install -m 0755 route      ${BASEDIR}/bin
	install -m 0755 slattach   $(BASEDIR)/sbin
ifeq ($(HAVE_IP_TOOLS),1)
	install -m 0755 ipmaddr    $(BASEDIR)/sbin
	install -m 0755 iptunnel   $(BASEDIR)/sbin
endif
ifeq ($(HAVE_MII),1)
	install -m 0755 mii-tool   $(BASEDIR)/sbin
endif
	ln -fs hostname $(BASEDIR)/bin/dnsdomainname
	ln -fs hostname $(BASEDIR)/bin/ypdomainname
	ln -fs hostname $(BASEDIR)/bin/nisdomainname
	ln -fs hostname $(BASEDIR)/bin/domainname
ifeq ($(HAVE_AFDECnet),1)
	ln -fs hostname $(BASEDIR)/bin/nodename
endif

savebin:
	@for i in ${BASEDIR}/sbin/arp ${BASEDIR}/sbin/ifconfig \
                 ${BASEDIR}/bin/netstat \
		 ${BASEDIR}/sbin/rarp ${BASEDIR}/sbin/route \
		 ${BASEDIR}/bin/hostname ${BASEDIR}/bin/ypdomainname \
                 ${BASEDIR}/bin/dnsdomainname ${BASEDIR}/bin/nisdomainname \
		 ${BASEDIR}/bin/domainname ; do \
		 [ -f $$i ] && cp -f $$i $$i.old ; done ; echo Saved.

installdata:
	$(MAKE) -C man install
	$(MAKE) -C po install

# End of Makefile.
