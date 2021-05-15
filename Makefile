CXXFLAGS ?= -O3 -flto -Wall
CXXFLAGS += $(shell llvm-config --cxxflags --link-static)
# this is only temporary and should be improved once the casting code is better
CXXFLAGS += -frtti
LDFLAGS  += $(shell llvm-config --ldflags --link-static --system-libs --libs all)

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
