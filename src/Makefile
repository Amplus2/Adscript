CXX ?= clang++
CXXFLAGS ?= -O3 -s -flto -Wall
CXXFLAGS += `llvm-config --cxxflags --ldflags --system-libs --libs all`

BUILD_DIR = ../build

all:
	mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) *.cc -o $(BUILD_DIR)/$(PREFIX)adscript
