SUBDIRS=	en_US.88591 fr_FR.88591 de_DE.88591
GENCATNEW=	gencat -new
GENCAT=		gencat

USE_NLS := $(shell grep -s 'define NLS 1' ../config.h)

ifneq ($(USE_NLS),)

all:	catalogs ../nettools-nls.h

else

all:
	@echo "Skipping NLS Generation!"

endif

../nettools-nls.h:	nettools.m
	${GENCATNEW} /dev/null nettools.m -h ../nettools-nls.h

nettools.m:	en_US.88591/nettools.cat

catalogs:	de_DE.88591/nettools.cat fr_FR.88591/nettools.cat \
		en_US.88591/nettools.cat

en_US.88591/nettools.cat:	en_US.88591/*.m
	@echo en_US.88591
	@cat en_US.88591/*.m > nettools.m
	@if [ -f en_US.88591/nettools.cat ]; then \
		${GENCAT} en_US.88591/nettools.cat nettools.m; \
	else \
		${GENCATNEW} en_US.88591/nettools.cat nettools.m; \
	fi

fr_FR.88591/nettools.cat:	fr_FR.88591/*.m
	@echo fr_FR.88591
	@cat fr_FR.88591/*.m > nettools.m
	@if [ -f fr_FR.88591/nettools.cat ]; then \
		${GENCAT} fr_FR.88591/nettools.cat nettools.m; \
	else \
		${GENCATNEW} fr_FR.88591/nettools.cat nettools.m; \
	fi

de_DE.88591/nettools.cat:	de_DE.88591/*.m
	@echo de_DE.88591
	@cat de_DE.88591/*.m > nettools.m
	@if [ -f de_DE.88591/nettools.cat ]; then \
		${GENCAT} de_DE.88591/nettools.cat nettools.m; \
	else \
		${GENCATNEW} de_DE.88591/nettools.cat nettools.m; \
	fi

clean:
	for i in ${SUBDIRS} ; \
	do \
	    rm -f $$i/*~ $$i/*.orig $$i/DEADJOE ; \
	    rm -f $$i/nettools.cat ; \
	done; \
	rm -f ../nettools-nls.h *~ *.orig DEADJOE nettools.m

clobber: clean
