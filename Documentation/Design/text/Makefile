ALL=overall.txt walkthrough.txt objects.txt
ALL_HTML=objects.html overall.html timeline.html walkthrough.html organisation.html	
ALL_LATEX=objects.latex overall.latex timeline.latex walkthrough.latex organisation.latex	

REST=/Users/jack/tmp/docutils/tools
REST2HTML=$(REST)/html.py
REST2LATEX=$(REST)/rst2latex.py
PDFLATEX=/usr/local/teTeX/bin/powerpc-apple-darwin-current/pdflatex
PYTHON=python

.SUFFIXES: .txt .html .tex .pdf

all: $(ALL_HTML) all.pdf

all.txt: $(ALL)
	cat $(ALL) > all.txt

.txt.html:
	$(PYTHON) $(REST2HTML) $< $@

.txt.tex:
	$(PYTHON) $(REST2LATEX) $< $@
	
.tex.pdf:
	$(PDFLATEX) $< 
	