CXXFLAGS ?= -O3 -Wall
CXXFLAGS += `llvm-config --cxxflags`
LDFLAGS  += `llvm-config --ldflags --system-libs --libs all` -flto -lLLVM

BUILD_DIR ?= build
EXE_NAME  ?= adscript
OUTPUT    ?= $(BUILD_DIR)/$(EXE_NAME)

SFLIES = $(wildcard src/*.cc)
OFLIES = $(SFLIES:.cc=.o)

all: $(OUTPUT)

$(OUTPUT): $(OFLIES)
	mkdir -p $(BUILD_DIR)
	clang++ $(LDFLAGS) $(OFLIES) -o $(OUTPUT)
	strip $(OUTPUT)

%.o: %.cc
	clang++ $(CXXFLAGS) -c $< -o $@

test: all
	cd $(BUILD_DIR) && ./$(EXE_NAME) ../examples/first.adscript
	cd $(BUILD_DIR) && ./$(EXE_NAME) ../examples/second.adscript

clean:
	rm -rf $(BUILD_DIR) $(OFLIES)

rebuild: clean all
