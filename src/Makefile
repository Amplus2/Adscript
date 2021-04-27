CXX_PRE_FLAGS ?= -O3 -s -flto -Wall
CXX_POST_FLAGS += `llvm-config --cxxflags --ldflags --system-libs --libs all`

BUILD_DIR = ../build

all:
	mkdir -p $(BUILD_DIR)
	clang++ $(CXX_PRE_FLAGS) *.cc $(CXX_POST_FLAGS) -o $(BUILD_DIR)/$(PREFIX)adscript
