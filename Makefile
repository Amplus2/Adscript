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

test: all
	cd $(BUILD_DIR) && ./$(EXE_NAME) -e -o first ../examples/first.adscript
	cd $(BUILD_DIR) && ./$(EXE_NAME) -e -o second ../examples/second.adscript
	cd $(BUILD_DIR) && ./$(EXE_NAME) -e -o hello-world ../examples/hello-world.adscript

clean:
	rm -rf $(BUILD_DIR) $(OFILES)

install: all
	cp -f $(OUTPUT) $(PREFIX)/bin/$(EXE_NAME)

reinstall: clean all
	cp -f $(OUTPUT) $(PREFIX)/bin/$(EXE_NAME)

# some guys will need it though
remove:
	sudo rm -f $(PREFIX)/bin/$(EXE_NAME)
