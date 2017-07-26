VERSION = 0.1

SRC = tt2.c

ifdef SYSTEMROOT
    EXE = tt2.exe
	LIBS = -lpdcurses
	RM = rm -f $(EXE)
	CLOC = cloc-1.56 --quiet
else
    EXE = tt2
	LIBS = -lcurses
	RM = rm -f $(EXE)
	CLOC = cloc --quiet
endif
	
all: sloc tt2

tt2: clean
	gcc $(SRC) $(LIBS) -DVERSION=\"${VERSION}\" -std=c99 -Wall -Wextra -pedantic -Os -o $(EXE)
	strip -s -R .comment -R .gnu.version -R .note -R .eh_frame -R .eh_frame_hdr $(EXE)

sloc:
	$(CLOC) $(SRC)

tt2.pdf:
	enscript $(SRC) --pretty-print=c --color=1 --no-header --output=- | ps2pdf - > tt2.pdf

clean:
	$(RM)

.PHONY: all sloc clean
