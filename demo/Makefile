#
# make test
#

SUBDIRS		=	hello-1 account random

all:	combatsh
	for dir in $(SUBDIRS) ; do \
		if (cd $$dir ; $(MAKE)); then true; else exit 1; fi; \
	done

clean:
	for dir in $(SUBDIRS) ; do \
		if (cd $$dir ; $(MAKE) clean); then true; else exit 1; fi; \
	done
	rm -rf core *~ *.o

distclean:	clean

combatsh:
	if test ! -f ../combatsh ; then \
		echo "error: build Combat before running the demos"; \
		exit 1; \
	fi
