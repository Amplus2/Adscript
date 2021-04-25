CXX = clang++
CXX_PRE_FLAGS = -O3
CXX_POST_FLAGS = -lLLVM `llvm-config --cxxflags --ldflags --system-libs --libs`
CXX_FILES = main.cc utils.cc ast.cc lexerparser.cc compiler.cc

OUT_DIR = ../build
OUT_NAME = adscript

default:
	mkdir -p $(OUT_DIR)
	$(CXX) $(CXX_PRE_FLAGS) $(CXX_FILES) $(CXX_POST_FLAGS) -o $(OUT_DIR)/$(OUT_NAME)
