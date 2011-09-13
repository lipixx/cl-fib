ELFNAME=compiler

all:    parser scanner compile

dbg:
	antlr -gs $(ELFNAME).g
	dlg parser.dlg scan.c
	gcc -I/usr/include/pccts -o $(ELFNAME) $(ELFNAME).c scan.c err.c

compile:
	gcc -I/usr/include/pccts -o $(ELFNAME) $(ELFNAME).c scan.c err.c

scanner:
	dlg parser.dlg scan.c

parser:
	antlr $(ELFNAME).g

clean:
	rm *.c *.h *.dlg *~ $(ELFNAME)

