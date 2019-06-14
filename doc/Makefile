LATEX    = pslatex
PDFTEX   = pdflatex
DVIPS    = dvips
FIG2DEV  = fig2dev

all: combat.ps

.SUFFIXES :
.SUFFIXES : .ltx .dvi .ps .pdf .fig .tex .eps

.ltx.dvi:
	$(LATEX) $<
	if grep -q Rerun `basename $< .ltx`.log || grep -q "No file" `basename $< .ltx`.log ; then $(LATEX) $< ; fi

.ltx.pdf:
	$(PDFTEX) $<
	if grep -q Rerun `basename $< .ltx`.log || grep -q "No file" `basename $< .ltx`.log ; then $(PDFTEX) $< ; fi

.dvi.ps:
	$(DVIPS) -f < $< > $@

.fig.tex:
	$(FIG2DEV) -L latex < $< > $@

.fig.eps:
	$(FIG2DEV) -L ps < $< > $@

clean:
	for fig in *.fig ; do \
		if test "$$fig" = "*.fig" ; then break; fi ; \
		rm -f `basename $$fig .fig`.eps ; \
		rm -f `basename $$fig .fig`.tex ; \
	done
	rm -f *.dvi *.o *.a *~ core
	rm -f *.aux *.log *.toc

distclean:	clean
	for ps in *.ps ; do \
		if test "$$ps" = "*.ps" ; then break; fi ; \
		if test -r `basename $$ps .ps`.ltx ; then \
			rm -f $$ps ; \
		fi ; \
	done
	for pdf in *.pdf ; do \
		if test "$$pdf" = "*.pdf" ; then break; fi ; \
		if test -r `basename $$pdf .pdf`.ltx ; then \
			rm -f $$pdf ; \
		fi ; \
	done
