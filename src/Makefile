CXXFLAGS ?= -O3 -s -flto -Wall -lLLVM
CXXFLAGS += `llvm-config --cxxflags --ldflags --system-libs --libs all`

BUILD_DIR = ../build

all:
	mkdir -p $(BUILD_DIR)
	clang++ $(CXXFLAGS) *.cc -o $(BUILD_DIR)/$(PREFIX)adscript

test: all
	cd $(BUILD_DIR) && ./$(PREFIX)adscript ../examples/first.adscript
	cd $(BUILD_DIR) && ./$(PREFIX)adscript ../examples/second.adscript
