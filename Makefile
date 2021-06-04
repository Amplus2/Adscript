CFLAGS   ?= -O3 -Wall
CXXFLAGS ?= -O3 -Wall
CXXFLAGS += `llvm-config --cxxflags`
LDFLAGS  += `llvm-config --ldflags --system-libs --libs all` -flto -lLLVM

EXE_NAME ?= adscript
OUTPUT   ?= ./$(EXE_NAME)

PREFIX ?= /usr/local

SFILES = $(wildcard src/*.cc)
OFILES = $(SFILES:.cc=.o)


LLVMV ?= $(shell llvm-config --version | cut -d. -f1)
NO_LLVM11 ?= $(shell test $(LLVMV) -eq "11"; echo $$?)

CXXFLAGS += -DNO_LLVM11=$(NO_LLVM11)

all: $(OUTPUT)

$(OUTPUT): $(OFILES)
	clang++ $(LDFLAGS) $(OFILES) -o $(OUTPUT)
	strip $(OUTPUT)

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

test: test/test.out test/bench.out
	test/test.out
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
