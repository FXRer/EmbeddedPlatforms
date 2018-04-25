VIEWER=evince
DOCNAME=tictactoe_paper

$(DOCNAME).pdf: $(DOCNAME).tex
	pdflatex $(DOCNAME).tex </dev/null
	pdflatex $(DOCNAME).tex </dev/null

view:
	evince $(DOCNAME).pdf &

clean:
	rm -f $(DOCNAME).aux $(DOCNAME).log
