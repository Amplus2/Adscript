CXXFLAGS ?= -O3 -s -flto -Wall
LLVMFLAGS = `llvm-config --cxxflags --ldflags --system-libs --libs all`

BUILD_DIR = ../build

all:
	mkdir -p $(BUILD_DIR)
	clang++ $(CXXFLAGS) *.cc $(LLVMFLAGS) -o $(BUILD_DIR)/$(PREFIX)adscript
