PREFIX ?= $(HOME)/.local

all: install

cppautomata: cgof.cpp
	g++ -O3 cgof.cpp -o cppautomata -lncursesw

install: cppautomata
	mkdir -p $(PREFIX)/bin $(PREFIX)/share/man/man1
	cp cppautomata $(PREFIX)/bin/
	cp cppautomata.1 $(PREFIX)/share/man/man1/

uninstall:
	rm -f $(PREFIX)/bin/cppautomata $(PREFIX)/share/man/man1/cppautomata.1

clean:
	rm -f cppautomata

.PHONY: all install uninstall clean
