#
# make test
#

SUBDIRS		=	1 2 3 4 5 6 7 8 9 10 11

all:	combatsh
	for dir in $(SUBDIRS) ; do \
		if (cd $$dir ; $(MAKE)); then true; else exit 1; fi; \
	done

test:	all
	for dir in $(SUBDIRS) ; do \
		if (cd $$dir ; $(MAKE) test); then true; else exit 1; fi; \
	done

clean:
	for dir in $(SUBDIRS) ; do \
		if (cd $$dir ; $(MAKE) clean); then true; else exit 1; fi; \
	done
	rm -rf core *~ *.o

purge:
	for dir in $(SUBDIRS) ; do \
		if (cd $$dir ; $(MAKE) purge); then true; else exit 1; fi; \
	done

distclean:	clean

combatsh:
	if test ! -f ../combatsh ; then \
		echo "error: build Combat before running the test suite"; \
		exit 1; \
	fi

