ALL=overall.txt walkthrough.txt objects.txt organisation.txt
ALL_HTML=objects.html overall.html timeline.html walkthrough.html organisation.html	
ALL_LATEX=objects.latex overall.latex walkthrough.latex organisation.latex	

REST=/Users/jack/tmp/docutils/tools
REST2HTML=$(REST)/html.py
REST2LATEX=$(REST)/rst2latex.py
PDFLATEX=/usr/local/teTeX/bin/powerpc-apple-darwin-current/pdflatex
MAKEINDEX=/usr/local/teTeX/bin/powerpc-apple-darwin-current/makeindex
PYTHON=python

.SUFFIXES: .txt .html .tex .pdf

all: $(ALL_HTML) all.pdf

clean:
	-rm all.txt all.tex all.aux all.idx all.idg all.ilg all.ind all.log all.out all.toc $(ALL_HTML) all.pdf
	
all.pdf: all.txt

all.txt: head_latex.txt $(ALL) diagrams.txt
	cat head_latex.txt $(ALL) diagrams.txt > all.txt

.txt.html:
	cat head_html.txt $< $*_tail.txt | $(PYTHON) $(REST2HTML) > $@

.txt.tex:
	$(PYTHON) $(REST2LATEX) --use-latex-toc=USE_LATEX_TOC $< $@
	
.tex.pdf:
	$(PDFLATEX) $< 
