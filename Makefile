CFLAGS   ?= -O3 -Wall
CXXFLAGS ?= -O3 -Wall
CXXFLAGS += `llvm-config --cxxflags`
LDFLAGS  += `llvm-config --ldflags --system-libs --libs all` -flto -lLLVM

BUILD_DIR ?= build
EXE_NAME  ?= adscript
OUTPUT    ?= $(BUILD_DIR)/$(EXE_NAME)

PREFIX ?= /usr/local

SFILES = $(wildcard src/*.cc)
OFILES = $(SFILES:.cc=.o)

all: $(OUTPUT)

$(OUTPUT): $(OFILES)
	mkdir -p $(BUILD_DIR)
	clang++ $(LDFLAGS) $(OFILES) -o $(OUTPUT)
	strip $(OUTPUT)

%.o: %.cc
	clang++ $(CXXFLAGS) -c $< -o $@

%.o: %.c
	clang $(CFLAGS) -c $< -o $@

%.o: %.adscript $(OUTPUT)
	$(OUTPUT) -o $@ $<

test/test: test/main.o test/basic.o
	clang test/*.o -o test/test

test: test/test
	test/test

clean:
	rm -rf $(BUILD_DIR) $(OFILES)

install: all
	cp -f $(OUTPUT) $(PREFIX)/bin/$(EXE_NAME)

# this should not exist
reinstall: clean install

uninstall:
	rm -f $(PREFIX)/bin/$(EXE_NAME)

.PHONY: all test clean install reinstall uninstall
