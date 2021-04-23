CXX = clang++
CXX_FLAGS = -Wall -Wextra
CXX_FILES = main.cc lexerparser.cc

OUT_DIR = ../build
OUT_NAME = adscript

default:
	mkdir -p $(OUT_DIR)
	$(CXX) $(CXX_FLAGS) $(CXX_FILES) -o $(OUT_DIR)/$(OUT_NAME)
