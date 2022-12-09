CFLAGS   ?= -O3 -Wall
CXXFLAGS ?= -O3 -Wall
CXXFLAGS += $(shell llvm-config --cxxflags)
LDFLAGS  += $(shell llvm-config --ldflags --system-libs --libs all) -flto -lLLVM

EXE_NAME ?= adscript
OUTPUT   ?= ./$(EXE_NAME)

PREFIX ?= /usr/local

SFILES = $(wildcard src/*.cc)
OFILES = $(SFILES:.cc=.o)

all: $(OUTPUT) compile_flags.txt SPEC.pdf

$(OUTPUT): $(OFILES)
	clang++ $(LDFLAGS) $(OFILES) -o $(OUTPUT)
	strip $(OUTPUT)

compile_flags.txt: Makefile
	echo '$(CXXFLAGS)' | tr ' ' '\n' > compile_flags.txt

%.o: %.cc
	clang++ $(CXXFLAGS) -c $< -o $@

%.o: %.c
	clang $(CFLAGS) -c $< -o $@

%.o: %.adscript $(OUTPUT)
	$(OUTPUT) -o $@ $<

test/test.out: test/main.o test/basic.o
	clang++ $^ -o $@

test/bench.out: test/lel.o test/bench.o
	clang++ $^ -o $@

%.pdf: %.md
	pandoc $< -o $@

test: test/test.out
	test/test.out

bench: test/bench.out
	test/bench.out

clean:
	rm -f $(OUTPUT) $(OFILES) test/*.o test/*.out

install: all
	cp -f $(OUTPUT) $(PREFIX)/bin/$(EXE_NAME)

# this should not exist
reinstall: clean install

uninstall:
	rm -f $(PREFIX)/bin/$(EXE_NAME)

.PHONY: all test clean install reinstall uninstall
