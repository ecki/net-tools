/* Copyright '97 by Andi Kleen. Subject to the GPL. */
/* 19980630 - i18n - Arnaldo Carvalho de Melo <acme@conectiva.com.br> */

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

int print_static; 

enum State { number = 0, i_forward, i_inp_icmp, i_outp_icmp, i_rto_alg };
#define normal number

struct entry {
	char *title;
	char *out; 
	enum State type;
}; 

static enum State state;
static int indent[] = { 4, 4, 8, 8, 4 };       /* for each state */

#define I_STATIC (1<<16) /* static configuration option. */
#define I_TITLE  (1<<17)

char *titles[] = { /* for each state */
	NULL, NULL, 
	N_("ICMP input histogram:"),
	N_("ICMP output histogram:"),
	NULL
};

/* XXX check against the snmp mib rfc.
 */
struct entry Iptab[] = {
	{ N_("Forwarding"), N_("Forwarding is %s"), i_forward|I_STATIC },
	{ N_("DefaultTTL"), N_("Default TTL is %d"), number|I_STATIC },
	{ N_("InReceives"), N_("%d total packets received"), number },
	{ N_("InHdrErrors"), N_("%d with invalid headers"), number },
	{ N_("InAddrErrors"), N_("%d with invalid addresses"), number },
	{ N_("ForwDatagrams"), N_("%d forwarded"), number },
	{ N_("InUnknownProtos"), N_("%d with unknown protocol"), number },
	{ N_("InDiscards"), N_("%d incoming packets discarded"), number },
	{ N_("InDelivers"), N_("%d incoming packets delivered"), number },
	{ N_("OutRequests"), N_("%d requests sent out"), number }, /*?*/
	{ N_("OutDiscards"), N_("%d outgoing packets dropped"), number }, 
	{ N_("OutNoRoutes"), N_("%d dropped because of missing route"), number },
	{ N_("ReasmTimeout"), N_("%d fragments dropped after timeout"), number },
	{ N_("ReasmReqds"), N_("%d reassemblies required"), number }, /* ? */
	{ N_("ReasmOKs"), N_("%d packets reassembled ok"), number }, 
	{ N_("ReasmFails"), N_("%d packet reassembles failed"), number }, 
	{ N_("FragOKs"), N_("%d fragments received ok"), number },
	{ N_("FragFails"), N_("%d fragments failed"), number },
	{ N_("FragCreates"), N_("%d fragments created"), number }
};

struct entry Icmptab[] = {
	{ N_("InMsgs"), N_("%d ICMP messages received"), number },
	{ N_("InErrors"), N_("%d input ICMP message failed."), number },
	{ N_("InDestUnreachs"), N_("destination unreachable: %d"), i_inp_icmp|I_TITLE },
	{ N_("InTimeExcds"), N_("timeout in transit: %d"), i_inp_icmp|I_TITLE },
	{ N_("InParmProbs"), N_("wrong parameters: %d"), i_inp_icmp|I_TITLE },  /*?*/
	{ N_("InSrcQuenchs"), N_("source quenchs: %d"), i_inp_icmp|I_TITLE },
	{ N_("InRedirects"), N_("redirects: %d"), i_inp_icmp|I_TITLE },
	{ N_("InEchos"), N_("echo requests: %d"), i_inp_icmp|I_TITLE },
	{ N_("InEchoReps"), N_("echo replies: %d"), i_inp_icmp|I_TITLE },
	{ N_("InTimestamps"), N_("timestamp request: %d"), i_inp_icmp|I_TITLE },
	{ N_("InTimestampReps"), N_("timestamp reply: %d"), i_inp_icmp|I_TITLE },
	{ N_("InAddrMasks"), N_("address mask request: %d"), i_inp_icmp|I_TITLE }, /*?*/
	{ N_("InAddrMaskReps"), N_("address mask replies"), i_inp_icmp|I_TITLE }, /*?*/
	{ N_("OutMsgs"), N_("%d ICMP messages sent"), number },
	{ N_("OutErrors"), N_("%d ICMP messages failed"), number },
	{ N_("OutDestUnreachs"), N_("destination unreachable: %d"), i_outp_icmp|I_TITLE },
	{ N_("OutTimeExcds"), N_("time exceeded: %d"), i_outp_icmp|I_TITLE },
	{ N_("OutParmProbs"), N_("wrong parameters: %d"), i_outp_icmp|I_TITLE }, /*?*/
	{ N_("OutSrcQuenchs"), N_("source quench: %d"), i_outp_icmp|I_TITLE },
	{ N_("OutRedirects"), N_("redirect: %d"), i_outp_icmp|I_TITLE },
	{ N_("OutEchos"), N_("echo request: %d"), i_outp_icmp|I_TITLE },
	{ N_("OutEchoReps"), N_("echo replies: %d"), i_outp_icmp|I_TITLE },
	{ N_("OutTimestamps"), N_("timestamp requests: %d"), i_outp_icmp|I_TITLE },
	{ N_("OutTimestampReps"), N_("timestamp replies: %d"), i_outp_icmp|I_TITLE },
	{ N_("OutAddrMasks"), N_("address mask requests: %d"), i_outp_icmp|I_TITLE },
	{ N_("OutAddrMaskReps"), N_("address mask replies: %d"), i_outp_icmp|I_TITLE },
};

struct entry Tcptab[] = {
	{ N_("RtoAlgorithm"), N_("RTO algorithm is %s"), i_rto_alg|I_STATIC },
	{ N_("RtoMin"), "", number },
	{ N_("RtoMax"), "", number },
	{ N_("MaxConn"), "", number },
	{ N_("ActiveOpens"), N_("%d active opens"), number },
	{ N_("PassiveOpens"), N_("%d passive opens"), number },
	{ N_("AttemptFails"), N_("%d failed connection attempts"), number },
	{ N_("EstabResets"), N_("%d connection resets received"), number },
	{ N_("CurrEstab"), N_("%d connections established"), number },
	{ N_("InSegs"), N_("%d segments received"), number },
	{ N_("OutSegs"), N_("%d segments send out"), number },
	{ N_("RetransSegs"), N_("%d segments retransmited"), number },
	{ N_("InErrs"), N_("%d bad segments received."), number },
	{ N_("OutRsts"), N_("%d resets sent"), number },
};

struct entry Udptab[] = {
	{ N_("InDatagrams"), N_("%d packets received"), number },
	{ N_("NoPorts"), N_("%d packets to unknown port received."), number },
	{ N_("InErrors"), N_("%d packet receive errors"), number },
	{ N_("OutDatagrams"), N_("%d packets send"), number },
};

struct tabtab {
	char *title; 
	struct entry *tab; 
	size_t size; 
}; 

struct tabtab snmptabs[] = { 
	{ "Ip", Iptab, sizeof(Iptab) },
	{ "Icmp", Icmptab, sizeof(Icmptab) },
	{ "Tcp", Tcptab, sizeof(Tcptab) },
	{ "Udp", Udptab, sizeof(Udptab) },
	{ NULL }
}; 

static char *skiptok(char *s)
{
	while (!isspace(*s) && *s != '\0')
		s++; 
	return s;
}


/* XXX IGMP */ 

int cmpentries(const void *a, const void *b)
{
	return strcmp( ((struct entry*)a)->title, ((struct entry*)b)->title);
}

void printval(struct tabtab *tab, char *title, int val) 
{
	struct entry *ent, key; 
	int type; 
	char buf[512];

	key.title = title; 
	ent = bsearch(&key, tab->tab, tab->size/sizeof(struct entry),
				  sizeof(struct entry), cmpentries); 
	if (!ent)  { /* try our best */ 
		printf("%*s%s: %d\n", indent[state], "", _(title), val);
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
			printf("%*s%s\n", indent[state], "", _(titles[type]));
	}

	buf[0] = '\0';
	switch (type) {
	case number:
		sprintf(buf, _(ent->out), val);
		break; 
	case i_forward:
		type = normal;
		sprintf(buf, _(ent->out), val == 2 ? _("enabled") : _("disabled"));
		break; 
	case i_outp_icmp:
	case i_inp_icmp: 
		if (val > 0) {
			sprintf(buf,_(ent->out), val); 
		}
		break; 
	case i_rto_alg: /* XXXX */
		break; 
	default:
		abort(); 
	}
	if (buf[0]) 
		printf("%*s%s\n",indent[type],"", buf);
	
	state = type;
}

struct tabtab *newtable(struct tabtab *tabs, char *title) 
{
	struct tabtab *t; 

	for (t = tabs; t->title; t++) 
		if (!strcmp(title, t->title)) {
			printf("%s:\n", _(title)); 
			state = normal; 
			return t; 
		}
	return NULL; 
}

void parsesnmp()
{
	FILE *f; 
	char buf1[512], buf2[512]; 
	char *sp, *np, *p; 

	f = fopen("/proc/net/snmp", "r"); 
	if (!f) {
		perror(_("cannot open /proc/net/snmp"));
		return;
	}
	while (fgets(buf1,sizeof buf1,f)) {
		int endflag; 
		struct tabtab *tab; 

		if (!fgets(buf2,sizeof buf2,f)) break; 
		sp = strchr(buf1, ':');
		np = strchr(buf2, ':'); 
		if (!np || !sp) 
			goto formaterr; 
		*sp = '\0'; 
		tab = newtable(snmptabs, buf1); 
		if (tab == NULL)  {
			UFWARN((printf(_("unknown title %s\n"), buf1)));
			continue; 
		}
		np++; sp++; 
		
		endflag = 0; 
		while (!endflag) {
			while(isspace(*sp)) sp++; 
			while(isspace(*np)) np++; 
			/*if (*np == '\0') goto formaterr;*/ 

			p = skiptok(sp); 
			if (*p == '\0') endflag=1; 
			*p = '\0'; 

			if (*sp != '\0') /* XXX */ 
				printval(tab, sp, strtoul(np,&np,10)); 
			sp = p+1; 
		}
	}
	if (ferror(f)) 
		perror("/proc/net/snmp"); 
	fclose(f); 
	return; 

formaterr: 
	perror(_("error parsing /proc/net/snmp")); 
	return; 
}

void inittab()
{
	struct tabtab *t;
 
	/* we sort at runtime because I'm lazy ;) */ 
	for (t = snmptabs; t->title; t++)  
		qsort(t->tab, t->size/sizeof(struct entry), 
			  sizeof(struct entry), cmpentries); 
}
