/* Copyright '97 by Andi Kleen. Subject to the GPL. */

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
	"ICMP input histogram:",
	"ICMP output histogram:",
	NULL
};

/* XXX check against the snmp mib rfc.
 */
struct entry Iptab[] = {
	{ "Forwarding", "Forwarding is %s", i_forward|I_STATIC },
	{ "DefaultTTL", "Default TTL is %d", number|I_STATIC },
	{ "InReceives", "%d total packets received", number },
	{ "InHdrErrors", "%d with invalid headers", number },
	{ "InAddrErrors", "%d with invalid addresses", number },
	{ "ForwDatagrams", "%d forwarded", number },
	{ "InUnknownProtos", "%d with unknown protocol", number },
	{ "InDiscards", "%d incoming packets discarded", number },
	{ "InDelivers", "%d incoming packets delivered", number },
	{ "OutRequests", "%d requests sent out", number }, /*?*/
	{ "OutDiscards", "%d outgoing packets dropped", number }, 
	{ "OutNoRoutes", "%d dropped because of missing route", number },
	{ "ReasmTimeout", "%d fragments dropped after timeout", number },
	{ "ReasmReqds", "%d reassemblies required", number }, /* ? */
	{ "ReasmOKs", "%d packets reassembled ok", number }, 
	{ "ReasmFails", "%d packet reassembles failed", number }, 
	{ "FragOKs", "%d fragments received ok", number },
	{ "FragFails", "%d fragments failed", number },
	{ "FragCreates", "%d fragments created", number }
};

struct entry Icmptab[] = {
	{ "InMsgs", "%d ICMP messages received", number },
	{ "InErrors", "%d input ICMP message failed.", number },
	{ "InDestUnreachs", "destination unreachable: %d", i_inp_icmp|I_TITLE },
	{ "InTimeExcds", "timeout in transit: %d", i_inp_icmp|I_TITLE },
	{ "InParmProbs", "wrong parameters: %d", i_inp_icmp|I_TITLE },  /*?*/
	{ "InSrcQuenchs", "source quenchs: %d", i_inp_icmp|I_TITLE },
	{ "InRedirects", "redirects: %d", i_inp_icmp|I_TITLE },
	{ "InEchos", "echo requests: %d", i_inp_icmp|I_TITLE },
	{ "InEchoReps", "echo replies: %d", i_inp_icmp|I_TITLE },
	{ "InTimestamps", "timestamp request: %d", i_inp_icmp|I_TITLE },
	{ "InTimestampReps", "timestamp reply: %d", i_inp_icmp|I_TITLE },
	{ "InAddrMasks", "address mask request: %d", i_inp_icmp|I_TITLE }, /*?*/
	{ "InAddrMaskReps", "address mask replies", i_inp_icmp|I_TITLE }, /*?*/
	{ "OutMsgs", "%d ICMP messages sent", number },
	{ "OutErrors", "%d ICMP messages failed", number },
	{ "OutDestUnreachs", "destination unreachable: %d", i_outp_icmp|I_TITLE },
	{ "OutTimeExcds", "time exceeded: %d", i_outp_icmp|I_TITLE },
	{ "OutParmProbs", "wrong parameters: %d", i_outp_icmp|I_TITLE }, /*?*/
	{ "OutSrcQuenchs", "source quench: %d", i_outp_icmp|I_TITLE },
	{ "OutRedirects", "redirect: %d", i_outp_icmp|I_TITLE },
	{ "OutEchos", "echo request: %d", i_outp_icmp|I_TITLE },
	{ "OutEchoReps", "echo replies: %d", i_outp_icmp|I_TITLE },
	{ "OutTimestamps", "timestamp requests: %d", i_outp_icmp|I_TITLE },
	{ "OutTimestampReps", "timestamp replies: %d", i_outp_icmp|I_TITLE },
	{ "OutAddrMasks", "address mask requests: %d", i_outp_icmp|I_TITLE },
	{ "OutAddrMaskReps", "address mask replies: %d", i_outp_icmp|I_TITLE },
};

struct entry Tcptab[] = {
	{ "RtoAlgorithm", "RTO algorithm is %s", i_rto_alg|I_STATIC },
	{ "RtoMin", "", number },
	{ "RtoMax", "", number },
	{ "MaxConn", "", number },
	{ "ActiveOpens", "%d active opens", number },
	{ "PassiveOpens", "%d passive opens", number },
	{ "AttemptFails", "%d failed connection attempts", number },
	{ "EstabResets", "%d connection resets received", number },
	{ "CurrEstab", "%d connections established", number },
	{ "InSegs", "%d segments received", number },
	{ "OutSegs", "%d segments send out", number },
	{ "RetransSegs", "%d segments retransmited", number },
	{ "InErrs", "%d bad segments received.", number },
	{ "OutRsts", "%d resets sent", number },
};

struct entry Udptab[] = {
	{ "InDatagrams", "%d packets received", number },
	{ "NoPorts", "%d packets to unknown port received.", number },
	{ "InErrors", "%d packet receive errors", number },
	{ "OutDatagrams", "%d packets send", number },
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
		printf("%*s%s: %d\n", indent[state], "", title, val);
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
			printf("%*s%s\n", indent[state], "", titles[type]);
	}

	buf[0] = '\0';
	switch (type) {
	case number:
		sprintf(buf, ent->out, val);
		break; 
	case i_forward:
		type = normal;
		sprintf(buf, ent->out, val == 2 ? "enabled" : "disabled");
		break; 
	case i_outp_icmp:
	case i_inp_icmp: 
		if (val > 0) {
			sprintf(buf,ent->out, val); 
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
			printf("%s:\n", title); 
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
		perror("cannot open /proc/net/snmp");
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
			UFWARN((printf("unknown title %s\n", buf1)));
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
	perror("error parsing /proc/net/snmp"); 
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
