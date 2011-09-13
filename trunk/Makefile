ELFNAME=compiler
ANTLROPT=-gs -gt
DLGOPT=-ci
CC=g++

all:    parser scanner compile

dbg:
	antlr $(ANTLROPT) $(ELFNAME).g
	dlg $(DLGOPT) parser.dlg scan.c
	$(CC) -I/usr/include/pccts -o $(ELFNAME) $(ELFNAME).c scan.c err.c

compile:
	$(CC) -I/usr/include/pccts -o $(ELFNAME) $(ELFNAME).c scan.c err.c

scanner:
	dlg $(DLGOPT) parser.dlg scan.c

parser:
	antlr $(ANTLROPT) $(ELFNAME).g

clean:
	rm *.c *.h *.dlg *~ $(ELFNAME)

