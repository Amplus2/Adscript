CXXFLAGS ?= -O3 -Wall
CXXFLAGS += `llvm-config --cxxflags`
LDFLAGS  += `llvm-config --ldflags --system-libs --libs all` -flto -lLLVM

BUILD_DIR ?= build
EXE_NAME  ?= adscript
OUTPUT    ?= $(BUILD_DIR)/$(EXE_NAME)

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

clean:
	rm -rf $(BUILD_DIR) $(OFILES)

rebuild: clean all

STATIC_LLVM_FLAGS = `llvm-config --cxxflags --ldflags --system-libs --link-static --libs all`
static:
	mkdir -p build
	clang++ $(SFILES) -flto -O3 -Wall $(STATIC_LLVM_FLAGS) -o build/adscript-static
	strip build/adscript-static
