VERSION = 1.0

SRC = tt2.c
PDCURSES ?= PDCurses-3.9

ifeq ($(OS),Windows_NT)
    EXE = tt2.exe
	LIBS = -L$(PDCURSES)\wincon -l:pdcurses.a 
	INCLUDE = -I$(PDCURSES)
	RM = wsl rm -f $(EXE)
	CLOC = cloc-2.02.exe --quiet
else
    EXE = tt2
	LIBS = -lcurses
	RM = rm -f $(EXE)
	CLOC = cloc --quiet
endif
	
all: tt2 sloc

tt2:
	gcc $(SRC) $(LIBS) $(INCLUDE) -DVERSION=\"${VERSION}\" -std=c99 -Wall -Wextra -pedantic -Os -o $(EXE)
	strip -s -R .comment -R .gnu.version -R .note -R .eh_frame -R .eh_frame_hdr $(EXE)

sloc:
	$(CLOC) $(SRC)

tt2.pdf:
	enscript $(SRC) --pretty-print=c --color=1 --no-header --output=- | ps2pdf - > tt2.pdf

clean:
	$(RM)

.PHONY: all sloc clean
