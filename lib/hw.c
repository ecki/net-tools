/*
 * lib/hw.c	This file contains the top-level part of the hardware
 *		support functions module for the NET-2 base distribution.
 *
 * Version:	lib/hw.c 1.20 (1998-01-25)
 *
 * Maintainer:	Bernd 'eckes' Eckenfels, <net-tools@lina.inka.de>
 *
 * Author:	Fred N. van Kempen, <waltje@uwalt.nl.mugnet.org>
 *		Copyright 1993 MicroWalt Corporation
 *
 *		This program is free software; you can redistribute it
 *		and/or  modify it under  the terms of  the GNU General
 *		Public  License as  published  by  the  Free  Software
 *		Foundation;  either  version 2 of the License, or  (at
 *		your option) any later version.
 */
#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <ctype.h>
#include <string.h>
#include <unistd.h>
#include "config.h"
#include "net-support.h"
#include "pathnames.h"
#define  EXTERN
#include "net-locale.h"


extern	struct hwtype	unspec_hwtype;
extern	struct hwtype	loop_hwtype;

extern	struct hwtype	slip_hwtype;
extern	struct hwtype	cslip_hwtype;
extern	struct hwtype	slip6_hwtype;
extern	struct hwtype	cslip6_hwtype;
extern	struct hwtype	adaptive_hwtype;

extern	struct hwtype	ether_hwtype;
extern	struct hwtype	fddi_hwtype;
extern	struct hwtype	hippi_hwtype;
extern	struct hwtype	tr_hwtype;

extern	struct hwtype	ax25_hwtype;
extern	struct hwtype	rose_hwtype;
extern  struct hwtype   netrom_hwtype;
extern  struct hwtype   tunnel_hwtype;

extern	struct hwtype	ash_hwtype;

extern	struct hwtype	ppp_hwtype;

extern	struct hwtype	arcnet_hwtype;

extern	struct hwtype	dlci_hwtype;
extern	struct hwtype	frad_hwtype;

extern	struct hwtype	hdlc_hwtype;
extern	struct hwtype	lapb_hwtype;

extern	struct hwtype	sit_hwtype;

static struct hwtype *hwtypes[] = {

  &loop_hwtype,

#if HAVE_HWSLIP
  &slip_hwtype,
  &cslip_hwtype,
  &slip6_hwtype,
  &cslip6_hwtype,
  &adaptive_hwtype,
#endif
#if HAVE_HWASH
  &ash_hwtype,
#endif
#if HAVE_HWETHER
  &ether_hwtype,
#endif
#if HAVE_HWTR
  &tr_hwtype,
#endif
#if HAVE_HWAX25
  &ax25_hwtype,
#endif
#if HAVE_HWNETROM
  &netrom_hwtype,
#endif
#if HAVE_HWTUNNEL
  &tunnel_hwtype,
#endif
#if HAVE_HWPPP
  &ppp_hwtype,
#endif  
#if HAVE_HWHDLCLAPB
  &hdlc_hwtype,
  &lapb_hwtype,
#endif
#if HAVE_HWARC
  &arcnet_hwtype,
#endif  
#if HAVE_HWFR
  &dlci_hwtype,
  &frad_hwtype,
#endif
#if HAVE_HWSIT
  &sit_hwtype,
#endif
#if HAVE_HWROSE
  &rose_hwtype,
#endif
#if HAVE_HWFDDI
  &fddi_hwtype,
#endif
#if HAVE_HWHIPPI
  &hippi_hwtype,
#endif
  &unspec_hwtype,
  NULL
};

static short sVhwinit = 0;

void hwinit ()
{
  loop_hwtype.title = NLS_CATSAVE (catfd, loopbackSet, loopback_loop, "Local Loopback");
  unspec_hwtype.title = NLS_CATSAVE (catfd, loopbackSet, loopback_unspec, "UNSPEC");
#if HAVE_HWSLIP
  slip_hwtype.title = NLS_CATSAVE (catfd, slipSet, slip_slip, "Serial Line IP");
  cslip_hwtype.title = NLS_CATSAVE (catfd, slipSet, slip_cslip, "VJ Serial Line IP");
  slip6_hwtype.title = NLS_CATSAVE (catfd, slipSet, slip_slip6, "6-bit Serial Line IP");
  cslip6_hwtype.title = NLS_CATSAVE (catfd, slipSet, slip_cslip6, "VJ 6-bit Serial Line IP");
  adaptive_hwtype.title = NLS_CATSAVE (catfd, slipSet, slip_adaptive, "Adaptive Serial Line IP");
#endif
#if HAVE_HWETHER
  ether_hwtype.title = NLS_CATSAVE (catfd, etherSet, ether_ether, "Ethernet");
#endif
#if HAVE_HWASH
  ash_hwtype.title = NLS_CATSAVE (catfd, ashSet, ash_hw, "Ash");
#endif
#if HAVE_HWFDDI
  fddi_hwtype.title = NLS_CATSAVE (catfd, fddiSet, fddi_fddi, "Fiber Distributed Data Interface");
#endif
#if HAVE_HWHIPPI
  hippi_hwtype.title = NLS_CATSAVE (catfd, hippiSet, hippi_hippi, "HIPPI");
#endif
#if HAVE_HWAX25
  ax25_hwtype.title = NLS_CATSAVE (catfd, ax25Set, ax25_hw, "AMPR AX.25");
#endif
#if HAVE_HWROSE
  rose_hwtype.title = NLS_CATSAVE (catfd, roseSet, rose_hw, "AMPR ROSE");
#endif
#if HAVE_HWNETROM
  netrom_hwtype.title = NLS_CATSAVE (catfd, netromSet, netrom_hw, "AMPR NET/ROM");
#endif
#if HAVE_HWTUNNEL
  tunnel_hwtype.title = NLS_CATSAVE (catfd, tunnelSet, tunnel_hw, "IPIP Tunnel");
#endif
#if HAVE_HWPPP
  ppp_hwtype.title = NLS_CATSAVE (catfd, pppSet, ppp_ppp, "Point-to-Point Protocol");
#endif  
#if HAVE_HWHDLCLAPB
  hdlc_hwtype.title = NLS_CATSAVE (catfd, hdlcSet, hdlc_hw, "(Cisco)-HDLC");
  lapb_hwtype.title = NLS_CATSAVE (catfd, lapbSet, lapb_hw, "LAPB");
#endif
#if HAVE_HWARC
  arcnet_hwtype.title = NLS_CATSAVE (catfd, arcnetSet, arcnet_arcnet, "1.5Mbps ARCnet");
#endif
#if HAVE_HWFR
  dlci_hwtype.title = NLS_CATSAVE(catfd, dlciSet, dlci_hw, "Frame Relay DLCI");
  frad_hwtype.title = NLS_CATSAVE(catfd, fradSet, frad_hw, "Frame Relay Access Device");
#endif
#if HAVE_HWSIT
  sit_hwtype.title = NLS_CATSAVE(catfd, sitSet, sit_hw, "IPv6-in-IPv4");
#endif
  sVhwinit = 1;
}

/* Check our hardware type table for this type. */
struct hwtype *
get_hwtype(const char *name)
{
  struct hwtype **hwp;

  if (!sVhwinit)
    hwinit();
  
  hwp = hwtypes;
  while (*hwp != NULL) {
	if (!strcmp((*hwp)->name, name)) return(*hwp);
	hwp++;
  }
  return(NULL);
}


/* Check our hardware type table for this type. */
struct hwtype *
get_hwntype(int type)
{
  struct hwtype **hwp;

  if (!sVhwinit)
    hwinit();

  hwp = hwtypes;
  while (*hwp != NULL) {
	if ((*hwp)->type == type) return(*hwp);
	hwp++;
  }
  return(NULL);
}
