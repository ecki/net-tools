/*
 * slattach	A program for handling dialup IP connecions.
 *		This program forces a TTY line to go into a special
 *		terminal line discipline, so that it can be used for
 *		network traffic instead of the regular terminal I/O.
 *
 * Usage:	slattach [-ehlmnqv] [ -k keepalive ] [ -o outfill ]
 * 			[-c cmd] [-s speed] [-p protocol] tty | -
 *
 * Version:	@(#)slattach.c	1.1.30  03/01/95
 *
 * Author:      Fred N. van Kempen, <waltje@uWalt.NL.Mugnet.ORG>
 *		Copyright 1988-1993 MicroWalt Corporation
 *
 * Modified:
 *		Alan Cox, <A.Cox@swansea.ac.uk> , July 16 1994
 *		Miquel van Smoorenburg, <miquels@drinkel.ow.org>, October 1994
 *		George Shearer, <gshearer@one.net>, January 3, 1995
 *		Yossi Gottlieb, <yogo@math.tau.ac.il>, February 11, 1995
 *		Peter Tobias, <tobias@et-inf.fho-emden.de>, July 30 1995
 *
 *		This program is free software; you can redistribute it
 *		and/or  modify it under  the terms of  the GNU General
 *		Public  License as  published  by  the  Free  Software
 *		Foundation;  either  version 2 of the License, or  (at
 *		your option) any later version.
 */
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <stdio.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <pwd.h>
#include <signal.h>
#include <stdlib.h>          
#include <string.h>
#include <unistd.h>
#include <linux/if_slip.h>

#if defined(__GLIBC__)
#if __GLIBC__ == 2 && __GLIBC_MINOR__ == 0
# include <termbits.h>
#else
# include <termios.h>
#endif
#endif

#include "pathnames.h"
#include "net-support.h"
#include "version.h"
#include "config.h"
#include "intl.h"

#ifndef _PATH_LOCKD
#define _PATH_LOCKD		"/var/lock"		/* lock files   */
#endif
#ifndef _UID_UUCP
#define _UID_UUCP		"uucp"			/* owns locks   */
#endif


#define DEF_PROTO	"cslip"


char *Release = RELEASE,
     *Version = "@(#) slattach 1.1.91 (12-Feb-95)",
     *Signature = "Fred N. van Kempen et al.";


struct {
  char	*speed;
  int	code;
} tty_speeds[] = {			/* table of usable baud rates	*/
  { "50",	B50	}, { "75",	B75  	},	
  { "110",	B110	}, { "300",	B300	},
  { "600",	B600	}, { "1200",	B1200	},
  { "2400",	B2400	}, { "4800",	B4800	},
  { "9600",	B9600	},
#ifdef B14400
  { "14400",	B14400	},
#endif
#ifdef B19200
  { "19200",	B19200	},
#endif
#ifdef B38400
  { "38400",	B38400	},
#endif
#ifdef B57600
  { "57600",	B57600	},
#endif
#ifdef B115200
  { "115200",	B115200	},
#endif
  { NULL,	0	}
};
struct termios	tty_saved,		/* saved TTY device state	*/
		tty_current;		/* current TTY device state	*/
int		tty_sdisc,		/* saved TTY line discipline	*/
		tty_ldisc,		/* current TTY line discipline	*/
		tty_fd = -1;		/* TTY file descriptor		*/
int		opt_c = 0;		/* "command" to run at exit	*/
int		opt_e = 0;		/* "activate only" flag		*/
int		opt_h = 0;		/* "hangup" on carrier loss	*/
#ifdef SIOCSKEEPALIVE
int		opt_k = 0;		/* "keepalive" value		*/
#endif
int		opt_l = 0;		/* "lock it" flag		*/
int		opt_L = 0;		/* clocal flag			*/
int		opt_m = 0;		/* "set RAW mode" flag		*/
int		opt_n = 0;		/* "set No Mesg" flag		*/
#ifdef SIOCSOUTFILL
int		opt_o = 0;		/* "outfill" value		*/
#endif
int		opt_q = 0;		/* "quiet" flag			*/
int		opt_d = 0;		/* debug flag			*/
int		opt_v = 0;		/* Verbose flag			*/

/* Disable any messages to the input channel of this process. */
static int
tty_nomesg(int fd)
{
  if (opt_n == 0) return(0);
  return(fchmod(fd, 0600));
}

/* Check for an existing lock file on our device */
static int
tty_already_locked(char *nam)
{
  int  i = 0, pid = 0;
  FILE *fd = (FILE *)0;

  /* Does the lock file on our device exist? */
  if ((fd = fopen(nam, "r")) == (FILE *)0)
    return(0); /* No, return perm to continue */

  /* Yes, the lock is there.  Now let's make sure */
  /* at least there's no active process that owns */
  /* that lock.                                   */
  i = fscanf(fd, "%d", &pid);
  (void) fclose(fd);
 
  if (i != 1) /* Lock file format's wrong! Kill't */
    return(0);

  /* We got the pid, check if the process's alive */
  if (kill(pid, 0) == 0)      /* it found process */
      return(1);          /* Yup, it's running... */

  /* Dead, we can proceed locking this device...  */
  return(0);
}

/* Lock or unlock a terminal line. */
static int
tty_lock(char *path, int mode)
{
  static char saved_path[PATH_MAX];
  static int saved_lock = 0;
  struct passwd *pw;
  int fd;
  char apid[16];

  /* We do not lock standard input. */
  if ((opt_l == 0) || ((path == NULL) && (saved_lock == 0))) return(0);

  if (mode == 1) {	/* lock */
	sprintf(saved_path, "%s/LCK..%s", _PATH_LOCKD, path);
	if (tty_already_locked(saved_path)) {
		fprintf(stderr, _("slattach: /dev/%s already locked!\n"), path);
		return(-1);
	}
	if ((fd = creat(saved_path, 0644)) < 0) {
		if (errno != EEXIST)
			if (opt_q == 0) fprintf(stderr,
				_("slattach: tty_lock: (%s): %s\n"),
					saved_path, strerror(errno));
		return(-1);
	}
	sprintf(apid, "%10d\n", getpid());
	if (write(fd, apid, strlen(apid)) != strlen(apid)) {
		fprintf(stderr, _("slattach: cannot write PID file\n"));
		close(fd);
		unlink(saved_path);
		return(-1);
	}

	(void) close(fd);

	/* Make sure UUCP owns the lockfile.  Required by some packages. */
	if ((pw = getpwnam(_UID_UUCP)) == NULL) {
		if (opt_q == 0) fprintf(stderr, _("slattach: tty_lock: UUCP user %s unknown!\n"),
					_UID_UUCP);
		return(0);	/* keep the lock anyway */
	}
	(void) chown(saved_path, pw->pw_uid, pw->pw_gid);
	saved_lock = 1;
  } else {	/* unlock */
	if (saved_lock != 1) return(0);
	if (unlink(saved_path) < 0) {
		if (opt_q == 0) fprintf(stderr,
			"slattach: tty_unlock: (%s): %s\n", saved_path,
							strerror(errno));
		return(-1);
	}
	saved_lock = 0;
  }

  return(0);
}


/* Find a serial speed code in the table. */
static int
tty_find_speed(char *speed)
{
  int i;

  i = 0;
  while (tty_speeds[i].speed != NULL) {
	if (!strcmp(tty_speeds[i].speed, speed)) return(tty_speeds[i].code);
	i++;
  }
  return(-EINVAL);
}


/* Set the number of stop bits. */
static int
tty_set_stopbits(struct termios *tty, char *stopbits)
{
  if (opt_d) printf("slattach: tty_set_stopbits: %c\n", *stopbits);
  switch(*stopbits) {
	case '1':
		tty->c_cflag &= ~CSTOPB;
		break;

	case '2':
		tty->c_cflag |= CSTOPB;
		break;

	default:
		return(-EINVAL);
  }
  return(0);
}


/* Set the number of data bits. */
static int
tty_set_databits(struct termios *tty, char *databits)
{
  if (opt_d) printf("slattach: tty_set_databits: %c\n", *databits);
  tty->c_cflag &= ~CSIZE;
  switch(*databits) {
	case '5':
		tty->c_cflag |= CS5;
		break;

	case '6':
		tty->c_cflag |= CS6;
		break;

	case '7':
		tty->c_cflag |= CS7;
		break;

	case '8':
		tty->c_cflag |= CS8;
		break;

	default:
		return(-EINVAL);
  }
  return(0);
}


/* Set the type of parity encoding. */
static int
tty_set_parity(struct termios *tty, char *parity)
{
  if (opt_d) printf("slattach: tty_set_parity: %c\n", *parity);
  switch(toupper(*parity)) {
	case 'N':
		tty->c_cflag &= ~(PARENB | PARODD);
		break;  

	case 'O':
		tty->c_cflag &= ~(PARENB | PARODD);
		tty->c_cflag |= (PARENB | PARODD);
		break;

	case 'E':
		tty->c_cflag &= ~(PARENB | PARODD);
		tty->c_cflag |= (PARENB);
		break;

	default:
		return(-EINVAL);
  }
  return(0);
}


/* Set the line speed of a terminal line. */
static int
tty_set_speed(struct termios *tty, char *speed)
{
  int code;

  if (opt_d) printf("slattach: tty_set_speed: %s\n", speed);
  if ((code = tty_find_speed(speed)) < 0) return(code);
  tty->c_cflag &= ~CBAUD;
  tty->c_cflag |= code;
  return(0);
}


/* Put a terminal line in a transparent state. */
static int
tty_set_raw(struct termios *tty)
{
  int i;
  int speed;

  for(i = 0; i < NCCS; i++)
		tty->c_cc[i] = '\0';		/* no spec chr		*/
  tty->c_cc[VMIN] = 1;
  tty->c_cc[VTIME] = 0;
  tty->c_iflag = (IGNBRK | IGNPAR);		/* input flags		*/
  tty->c_oflag = (0);				/* output flags		*/
  tty->c_lflag = (0);				/* local flags		*/
  speed = (tty->c_cflag & CBAUD);		/* save current speed	*/
  tty->c_cflag = (CRTSCTS | HUPCL | CREAD);	/* UART flags		*/
  if (opt_L) 
	tty->c_cflag |= CLOCAL;
  tty->c_cflag |= speed;			/* restore speed	*/
  return(0);
}


/* Fetch the state of a terminal. */
static int
tty_get_state(struct termios *tty)
{
  if (ioctl(tty_fd, TCGETS, tty) < 0) {
	if (opt_q == 0) fprintf(stderr,
		"slattach: tty_get_state: %s\n", strerror(errno));
	return(-errno);
  }
  return(0);
}


/* Set the state of a terminal. */
static int
tty_set_state(struct termios *tty)
{
  if (ioctl(tty_fd, TCSETS, tty) < 0) {
	if (opt_q == 0) fprintf(stderr,
		"slattach: tty_set_state: %s\n", strerror(errno));
	return(-errno);
  }
  return(0);
}


/* Get the line discipline of a terminal line. */
static int
tty_get_disc(int *disc)
{
  if (ioctl(tty_fd, TIOCGETD, disc) < 0) {
	if (opt_q == 0) fprintf(stderr,
		"slattach: tty_get_disc: %s\n", strerror(errno));
	return(-errno);
  }
  return(0);
}


/* Set the line discipline of a terminal line. */
static int
tty_set_disc(int disc)
{
  if (disc == -1) disc = tty_sdisc;

  if (ioctl(tty_fd, TIOCSETD, &disc) < 0) {
	if (opt_q == 0) fprintf(stderr,
		"slattach: tty_set_disc(%d, %d): %s\n", tty_fd,
			disc, strerror(errno));
	return(-errno);
  }
  return(0);
}


/* Fetch the name of the network interface attached to this terminal. */
static int
tty_get_name(char *name)
{
  if (ioctl(tty_fd, SIOCGIFNAME, name) < 0) {
	if (opt_q == 0) fprintf(stderr,
		"slattach: tty_get_name: %s\n", strerror(errno));
	return(-errno);
  }
  return(0);
}


/* Hangup the line. */
static int
tty_hangup(void)
{
  struct termios tty;

  tty = tty_current;
  (void) tty_set_speed(&tty, "0");
  if (tty_set_state(&tty) < 0) {
	if (opt_q == 0) fprintf(stderr, _("slattach: tty_hangup(DROP): %s\n"), strerror(errno));
	return(-errno);
  }

  (void) sleep(1);

  if (tty_set_state(&tty_current) < 0) {
	if (opt_q == 0) fprintf(stderr, _("slattach: tty_hangup(RAISE): %s\n"), strerror(errno));
	return(-errno);
  }
  return(0);
}


/* Close down a terminal line. */
static int
tty_close(void)
{
  (void) tty_set_disc(tty_sdisc);
  (void) tty_hangup();
  (void) tty_lock(NULL, 0);
  return(0);
}


/* Open and initialize a terminal line. */
static int
tty_open(char *name, char *speed)
{
  char path[PATH_MAX];
  register char *sp;
  int fd;

  /* Try opening the TTY device. */
  if (name != NULL) {
	if ((sp = strrchr(name, '/')) != (char *)NULL) *sp++ = '\0';
	  else sp = name;
	sprintf(path, "/dev/%s", sp);
	if (tty_lock(sp, 1)) return(-1); /* can we lock the device? */
	if ((fd = open(path, O_RDWR)) < 0) {
		if (opt_q == 0) fprintf(stderr,
			"slattach: tty_open(%s, RW): %s\n",
					path, strerror(errno));
		return(-errno);
	}
	tty_fd = fd;
	if (opt_d) printf("slattach: tty_open: %s (%d) ", path, fd);
  } else {
	tty_fd = 0;
	sp = (char *)NULL;
  }

  /* Fetch the current state of the terminal. */
  if (tty_get_state(&tty_saved) < 0) {
	if (opt_q == 0) fprintf(stderr, _("slattach: tty_open: cannot get current state!\n"));
	return(-errno);
  }
  tty_current = tty_saved;

  /* Fetch the current line discipline of this terminal. */
  if (tty_get_disc(&tty_sdisc) < 0) {
	if (opt_q == 0) fprintf(stderr, _("slattach: tty_open: cannot get current line disc!\n"));
	return(-errno);
  } 
  tty_ldisc = tty_sdisc;

  /* Put this terminal line in a 8-bit transparent mode. */
  if (opt_m == 0) {
	if (tty_set_raw(&tty_current) < 0) {
		if (opt_q == 0) fprintf(stderr, _("slattach: tty_open: cannot set RAW mode!\n"));
		return(-errno);
	}

	/* Set the default speed if we need to. */
	if (speed != NULL) {
		if (tty_set_speed(&tty_current, speed) != 0) {
			if (opt_q == 0) fprintf(stderr, _("slattach: tty_open: cannot set %s bps!\n"),
						speed);
			return(-errno);
		}
	}

	/* Set up a completely 8-bit clean line. */
	if (tty_set_databits(&tty_current, "8") ||
	    tty_set_stopbits(&tty_current, "1") ||
	    tty_set_parity(&tty_current, "N")) {
		if (opt_q == 0) fprintf(stderr, _("slattach: tty_open: cannot set 8N1 mode!\n"));
		return(-errno);
  	}

	/* Set the new line mode. */
	if ((fd = tty_set_state(&tty_current)) < 0) return(fd);
  }

  /* OK, line is open.  Do we need to "silence" it? */
  (void) tty_nomesg(tty_fd);

  return(0);
}


/* Catch any signals. */
static void
sig_catch(int sig)
{
/*  (void) signal(sig, sig_catch); */
  tty_close();
  exit(0);
}


static void
usage(void)
{
  char *usage_msg = "Usage: slattach [-ehlLmnqv] "
#ifdef SIOCSKEEPALIVE
	  "[-k keepalive] "
#endif
#ifdef SIOCSOUTFILL
	  "[-o outfill] "
#endif
	  "[-c cmd] [-s speed] [-p protocol] tty | -\n"
	  "       slattach -V\n";

  fprintf(stderr, usage_msg);
  exit(1);
}


static void 
version(void)
{
    printf("%s\n%s\n%s\n", Release, Version, Signature);
    exit(E_VERSION);
}


int
main(int argc, char *argv[])
{
  char path[128];
  char buff[128];
  char *speed = NULL;
  char *proto = DEF_PROTO;
  char *extcmd = (char *)0;
  struct hwtype *ht;
  char *sp;
  int s;

  strcpy(path, "");

  /* Scan command line for any arguments. */
  opterr = 0;
  while ((s = getopt(argc, argv, "c:ehlLmnp:qs:vdV"
#ifdef SIOCSKEEPALIVE
		     "k:"
#endif
#ifdef SIOCSOUTFILL
		     "o:"
#endif
		     )) != EOF) switch(s) {
	case 'c':
		extcmd = optarg;
		break;

	case 'e':
		opt_e = 1 - opt_e;
		break;

	case 'h':
		opt_h = 1 - opt_h;
		break;

#ifdef SIOCSKEEPALIVE
	case 'k':
		opt_k = atoi(optarg);
		break;
#endif

	case 'L':
		opt_L = 1 - opt_L;
		break;

	case 'l':
		opt_l = 1 - opt_l;
		break;

	case 'm':
		opt_m = 1 - opt_m;
		break;

	case 'n':
		opt_n = 1 - opt_n;
		break;

#ifdef SIOCSOUTFILL
	case 'o':
		opt_o = atoi(optarg);
		break;
#endif

	case 'p':
		proto = optarg;
		break;

	case 'q':
		opt_q = 1 - opt_q;
		break;

	case 's':
		speed = optarg;
		break;

	case 'd':
		opt_d = 1 - opt_d;
		break;

	case 'v':
		opt_v = 1 - opt_v;
		break;

        case 'V':
		version();
		/*NOTREACHED*/

	default:
		usage();
		/*NOTREACHED*/
  }
  
  activate_init();

  /* Check the protocol. */
  if ((ht = get_hwtype(proto)) == NULL && strcmp(proto, "tty")) {
	if (opt_q == 0) fprintf(stderr, _("slattach: unsupported protocol %s\n"), proto);
	return(2);
  }
  if (ht == NULL) opt_m++;

  /* Is a terminal given? */
  if (optind != (argc - 1)) usage();
  strncpy(path, argv[optind], 128);
  if (!strcmp(path, "-")) {
	opt_e = 1;
	sp = NULL;
	if (tty_open(NULL, speed) < 0) { return(3); }
  } else {
	if ((sp = strrchr(path, '/')) != NULL) *sp++ = '\0';
	  else sp = path;
	if (tty_open(sp, speed) < 0) { return(3); }
  }

  /* Start the correct protocol. */
  if (ht == NULL) {
	tty_sdisc = N_TTY;
	tty_close();
	return(0);
  }
  (*ht->activate)(tty_fd);
  if (opt_v == 1) {
	tty_get_name(buff);
	printf(_("%s started"), proto);
	if (sp != NULL) printf(_(" on %s"), sp);
	printf(_(" interface %s\n"), buff);
  }

  /* Configure keepalive and outfill. */
#ifdef SIOCSKEEPALIVE
  if (opt_k && (ioctl(tty_fd, SIOCSKEEPALIVE, &opt_k) < 0))
	  fprintf(stderr, "slattach: ioctl(SIOCSKEEPALIVE): %s\n", strerror(errno));
#endif
#ifdef SIOCSOUTFILL
  if (opt_o && (ioctl(tty_fd, SIOCSOUTFILL, &opt_o) < 0))
	  fprintf(stderr, "slattach: ioctl(SIOCSOUTFILL): %s\n", strerror(errno));
#endif

  (void) signal(SIGHUP, sig_catch);
  (void) signal(SIGINT, sig_catch);
  (void) signal(SIGQUIT, sig_catch);
  (void) signal(SIGTERM, sig_catch);

  /* Wait until we get killed if hanging on a terminal. */
  if (opt_e == 0) {
	while(1) {
		if(opt_h == 1) { /* hangup on carrier loss */
			int n = 0;

		        ioctl(tty_fd, TIOCMGET, &n);
			if(!(n & TIOCM_CAR))
				break;
			sleep(15);
		}
		else
			sleep(60);
	};

	tty_close();
	if(extcmd!=(char *)0) /* external command on exit */
		system(extcmd);
  }
  exit(0);
}