CXX = clang++
CXX_FLAGS = -Wextra
CXX_FILES = main.cc

default:
	$(CXX) $(CXX_FLAGS) $(CXX_FILES)
