/* Copyright 1997,1999 by Andi Kleen. Subject to the GPL. */
/* $Id: statistics.c,v 1.11 1999/01/06 12:05:49 freitag Exp $ */ 
/* 19980630 - i18n - Arnaldo Carvalho de Melo <acme@conectiva.com.br> */
/* 19981113 - i18n fixes - Arnaldo Carvalho de Melo <acme@conectiva.com.br> */
/* 19990101 - added net/netstat, -t, -u, -w supprt - Bernd Eckenfels */
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "config.h"
#include "intl.h"

/* #define WARN 1 */

#ifdef WARN
#define UFWARN(x) x
#else
#define UFWARN(x)
#endif

int print_static,f_raw,f_tcp,f_udp;

enum State {
    number = 0, opt_number, i_forward, i_inp_icmp, i_outp_icmp, i_rto_alg,
    MaxState
};

#define normal number

struct entry {
    char *title;
    char *out;
    enum State type;
};

struct statedesc { 
    int indent;
    char *title; 
}; 

struct statedesc states[] = { 
    [number] = { 4, NULL },
    [opt_number] = { 4, NULL }, 
    [i_forward] = { 4, NULL },
    [i_inp_icmp] = { 8, N_("ICMP input histogram:") },
    [i_outp_icmp] = { 8, N_("ICMP output histogram:") },
    [MaxState] = {0},
}; 

static enum State state;

#define I_STATIC (1<<16)	/* static configuration option. */
#define I_TITLE  (1<<17)

/* 
 * XXX check against the snmp mib rfc.
 *
 * Don't mark the first field as translatable! It's a snmp MIB standard.
 * - acme
 */
struct entry Iptab[] =
{
    {"Forwarding", N_("Forwarding is %s"), i_forward | I_STATIC},
    {"DefaultTTL", N_("Default TTL is %d"), number | I_STATIC},
    {"InReceives", N_("%d total packets received"), number},
    {"InHdrErrors", N_("%d with invalid headers"), opt_number},
    {"InAddrErrors", N_("%d with invalid addresses"), opt_number},
    {"ForwDatagrams", N_("%d forwarded"), number},
    {"InUnknownProtos", N_("%d with unknown protocol"), opt_number},
    {"InDiscards", N_("%d incoming packets discarded"), number},
    {"InDelivers", N_("%d incoming packets delivered"), number},
    {"OutRequests", N_("%d requests sent out"), number},	/*? */
    {"OutDiscards", N_("%d outgoing packets dropped"), opt_number},
    {"OutNoRoutes", N_("%d dropped because of missing route"), opt_number},
    {"ReasmTimeout", N_("%d fragments dropped after timeout"), opt_number},
    {"ReasmReqds", N_("%d reassemblies required"), opt_number},	/* ? */
    {"ReasmOKs", N_("%d packets reassembled ok"), opt_number},
    {"ReasmFails", N_("%d packet reassembles failed"), opt_number},
    {"FragOKs", N_("%d fragments received ok"), opt_number},
    {"FragFails", N_("%d fragments failed"), opt_number},
    {"FragCreates", N_("%d fragments created"), opt_number}
};

struct entry Icmptab[] =
{
    {"InMsgs", N_("%d ICMP messages received"), number},
    {"InErrors", N_("%d input ICMP message failed."), number},
    {"InDestUnreachs", N_("destination unreachable: %d"), i_inp_icmp | I_TITLE},
    {"InTimeExcds", N_("timeout in transit: %d"), i_inp_icmp | I_TITLE},
    {"InParmProbs", N_("wrong parameters: %d"), i_inp_icmp | I_TITLE},	/*? */
    {"InSrcQuenchs", N_("source quenchs: %d"), i_inp_icmp | I_TITLE},
    {"InRedirects", N_("redirects: %d"), i_inp_icmp | I_TITLE},
    {"InEchos", N_("echo requests: %d"), i_inp_icmp | I_TITLE},
    {"InEchoReps", N_("echo replies: %d"), i_inp_icmp | I_TITLE},
    {"InTimestamps", N_("timestamp request: %d"), i_inp_icmp | I_TITLE},
    {"InTimestampReps", N_("timestamp reply: %d"), i_inp_icmp | I_TITLE},
    {"InAddrMasks", N_("address mask request: %d"), i_inp_icmp | I_TITLE},	/*? */
    {"InAddrMaskReps", N_("address mask replies"), i_inp_icmp | I_TITLE},	/*? */
    {"OutMsgs", N_("%d ICMP messages sent"), number},
    {"OutErrors", N_("%d ICMP messages failed"), number},
    {"OutDestUnreachs", N_("destination unreachable: %d"), i_outp_icmp | I_TITLE},
    {"OutTimeExcds", N_("time exceeded: %d"), i_outp_icmp | I_TITLE},
    {"OutParmProbs", N_("wrong parameters: %d"), i_outp_icmp | I_TITLE},	/*? */
    {"OutSrcQuenchs", N_("source quench: %d"), i_outp_icmp | I_TITLE},
    {"OutRedirects", N_("redirect: %d"), i_outp_icmp | I_TITLE},
    {"OutEchos", N_("echo request: %d"), i_outp_icmp | I_TITLE},
    {"OutEchoReps", N_("echo replies: %d"), i_outp_icmp | I_TITLE},
    {"OutTimestamps", N_("timestamp requests: %d"), i_outp_icmp | I_TITLE},
    {"OutTimestampReps", N_("timestamp replies: %d"), i_outp_icmp | I_TITLE},
    {"OutAddrMasks", N_("address mask requests: %d"), i_outp_icmp | I_TITLE},
    {"OutAddrMaskReps", N_("address mask replies: %d"), i_outp_icmp | I_TITLE},
};

struct entry Tcptab[] =
{
    {"RtoAlgorithm", N_("RTO algorithm is %s"), i_rto_alg | I_STATIC},
    {"RtoMin", "", number},
    {"RtoMax", "", number},
    {"MaxConn", "", number},
    {"ActiveOpens", N_("%d active connections openings"), number},
    {"PassiveOpens", N_("%d passive connection openings"), number},
    {"AttemptFails", N_("%d failed connection attempts"), number},
    {"EstabResets", N_("%d connection resets received"), number},
    {"CurrEstab", N_("%d connections established"), number},
    {"InSegs", N_("%d segments received"), number},
    {"OutSegs", N_("%d segments send out"), number},
    {"RetransSegs", N_("%d segments retransmited"), number},
    {"InErrs", N_("%d bad segments received."), number},
    {"OutRsts", N_("%d resets sent"), number},
};

struct entry Udptab[] =
{
    {"InDatagrams", N_("%d packets received"), number},
    {"NoPorts", N_("%d packets to unknown port received."), number},
    {"InErrors", N_("%d packet receive errors"), number},
    {"OutDatagrams", N_("%d packets sent"), number},
};

struct entry Tcpexttab[] =
{
    {"SyncookiesSent", N_("%d SYN cookies sent"), opt_number},
    {"SyncookiesRecv", N_("%d SYN cookies received"), opt_number},
    {"SyncookiesFailed", N_("%d invalid SYN cookies received"), opt_number},

    { "EmbryonicRsts", N_("%d resets received for embryonic SYN_RECV sockets"),
      opt_number },  
    { "PruneCalled", N_("%d packets pruned from receive queue because of socket"
			" buffer overrun"), opt_number },  
    /* obsolete: 2.2.0 doesn't do that anymore */
    { "RcvPruned", N_("%d packets pruned from out-of-order queue"), opt_number },
    { "OfoPruned", N_("%d packets dropped from out-of-order queue because of"
		      " socket buffer overrun"), opt_number }, 
    { "OutOfWindowIcmps", N_("%d ICMP packets dropped because they were "
			     "out-of-window"), opt_number }, 
    { "LockDroppedIcmps", N_("%d ICMP packets dropped because socket was locked"),
      opt_number }, 
};

struct tabtab {
    char *title;
    struct entry *tab;
    size_t size;
    int *flag; 
};

struct tabtab snmptabs[] =
{
    {"Ip", Iptab, sizeof(Iptab), &f_raw},
    {"Icmp", Icmptab, sizeof(Icmptab), &f_raw},
    {"Tcp", Tcptab, sizeof(Tcptab), &f_tcp},
    {"Udp", Udptab, sizeof(Udptab), &f_udp},
    {"TcpExt", Tcpexttab, sizeof(Tcpexttab), &f_tcp},
    {NULL}
};

/* XXX IGMP */

int cmpentries(const void *a, const void *b)
{
    return strcmp(((struct entry *) a)->title, ((struct entry *) b)->title);
}

void printval(struct tabtab *tab, char *title, int val)
{
    struct entry *ent, key;
    int type;
    char buf[512];

    key.title = title;
    ent = bsearch(&key, tab->tab, tab->size / sizeof(struct entry),
		  sizeof(struct entry), cmpentries);
    if (!ent) {			/* try our best */
	printf("%*s%s: %d\n", states[state].indent, "", title, val);
	return;
    }
    type = ent->type;
    if (type & I_STATIC) {
	type &= ~I_STATIC;
	if (!print_static)
	    return;
    }
    if (*ent->out == '\0')
	return;

    if (type & I_TITLE) {
	type &= ~I_TITLE;
	if (state != type)
	    printf("%*s%s\n", states[state].indent, "", _(states[type].title));
    }
    buf[0] = '\0';
    switch (type) {
    case opt_number:
	if (val == 0) 
	    break;
	/*FALL THOUGH*/
    case number:
	snprintf(buf, sizeof(buf), _(ent->out), val);
	break;
    case i_forward:
	type = normal;
	snprintf(buf, sizeof(buf), _(ent->out), val == 2 ? _("enabled") : _("disabled"));
	break;
    case i_outp_icmp:
    case i_inp_icmp:
	if (val > 0)
	    snprintf(buf, sizeof(buf), _(ent->out), val);
	break;
    case i_rto_alg:		/* XXXX */
	break;
    default:
	abort();
    }
    if (buf[0])
	printf("%*s%s\n", states[type].indent, "", buf);

    state = type;
}

struct tabtab *newtable(struct tabtab *tabs, char *title)
{
    struct tabtab *t;

    for (t = tabs; t->title; t++)
	if (!strcmp(title, t->title)) {
	    if (*(t->flag))
		printf("%s:\n", _(title));
	    state = normal;
	    return t;
	}
    return NULL;
}


void process_fd(FILE *f)
{
    char buf1[512], buf2[512];
    char *sp, *np, *p;
    while (fgets(buf1, sizeof buf1, f)) {
	int endflag;
	struct tabtab *tab;

	if (!fgets(buf2, sizeof buf2, f))
	    break;
	sp = strchr(buf1, ':');
	np = strchr(buf2, ':');
	if (!np || !sp)
	    goto formaterr;
	*sp = '\0';
	tab = newtable(snmptabs, buf1);
	if (tab == NULL) {
	    UFWARN((printf(_("unknown title %s\n"), buf1)));
	    continue;
	}
	np++;
	sp++;

	endflag = 0;
	while (!endflag) {
	    sp += strspn(sp, " \t\n"); 
	    np += strspn(np, " \t\n"); 
	    /*if (*np == '\0') goto formaterr; */

	    p = sp+strcspn(sp, " \t\n");
	    if (*p == '\0')
		endflag = 1;
	    *p = '\0';

	    if (*sp != '\0' && *(tab->flag)) 	
		printval(tab, sp, strtoul(np, &np, 10));

	    sp = p + 1;
	}
    }
  return;
  
formaterr:
  perror(_("error parsing /proc/net/snmp"));
  return;
}


void parsesnmp(int flag_raw, int flag_tcp, int flag_udp)
{
    FILE *f;

    f_raw = flag_raw; f_tcp = flag_tcp; f_udp = flag_udp;
    
    f = fopen("/proc/net/snmp", "r");
    if (!f) {
	perror(_("cannot open /proc/net/snmp"));
	return;
    }
    process_fd(f);

    if (ferror(f))
	perror("/proc/net/snmp");

    fclose(f);

    f = fopen("/proc/net/netstat", "r");

    if (f) {
    	process_fd(f);

        if (ferror(f))
	    perror("/proc/net/netstat");
    
        fclose(f);
    }
    return;
}
    

void inittab(void)
{
    struct tabtab *t;

    /* we sort at runtime because I'm lazy ;) */
    for (t = snmptabs; t->title; t++)
	qsort(t->tab, t->size / sizeof(struct entry),
	      sizeof(struct entry), cmpentries);
}
