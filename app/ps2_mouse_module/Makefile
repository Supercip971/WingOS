PROJECT_NAME := ps2_mouse_module
DIRECTORY_GUARD=mkdir -p $(@D)

LIBS_DIR := ../../libs/
BUILD_DIR := ../../build/
PROJECT_DIRS := . $(LIBS_DIR)
CFILES := $(shell find $(PROJECT_DIRS) -type f -name \*.c)
CXXFILES := $(shell find $(PROJECT_DIRS) -type f -name \*.cpp)
HFILES := $(shell find $(PROJECT_DIRS) -type f -name \*.h)

CXXOBJ := $(patsubst %.cpp,$(BUILD_DIR)/%.io,$(CXXFILES) )
COBJ := $(patsubst %.c,$(BUILD_DIR)/%.io, $(CFILES))
CXX = ../../cross_compiler/bin/x86_64-pc-wingos-g++
CC = ../../cross_compiler/bin/x86_64-pc-wingos-gcc
LD = ../../cross_compiler/bin/x86_64-pc-wingos-ld
CXXHARDFLAGS := $(CFLAGS)               \
        -DBUILD_TIME='"$(BUILD_TIME)"' \
        -std=c++20                     \
        -g \
        -masm=intel                    \
        -DMODULE \
        -fpic \
        -fpie \
        -pie \
        -m64 \
        -O3 \
        -Werror \
        -msse \
        -mmmx \
        -mno-avx512f \
        -mcmodel=large \
        -mno-80387                     \
        -mno-red-zone                  \
        -fno-rtti \
        -fno-exceptions \
		-ffreestanding                 \
        -I$(LIBS_DIR) -I$(LIBS_DIR)/libc -I$(LIBS_DIR)/utils \

CHARDFLAGS := $(CFLAGS)               \
        -DBUILD_TIME='"$(BUILD_TIME)"' \
        -std=c11                     \
        -g \
        -masm=intel                    \
        -DMODULE \
        -fpic \
        -fpie \
        -pie \
        -m64 \
        -O3 \
        -Werror \
        -msse \
        -mmmx \
        -mno-avx512f \
        -mcmodel=large \
        -mno-80387                     \
        -mno-red-zone                  \
        -fno-rtti \
        -fno-exceptions \
		-ffreestanding                 \
        -I$(LIBS_DIR) -I$(LIBS_DIR)/libc -I$(LIBS_DIR)/utils \

all: $(CXXOBJ) $(COBJ)
	@$(LD) -nostdlib -pie --init=__entry_point -fini=__end_point -o $(PROJECT_NAME).module $(COBJ) $(CXXOBJ)
	@$(LD) -nostdlib -pie --init=__entry_point -fini=__end_point -o ../../initfs/$(PROJECT_NAME).module  $(CXXOBJ)


$(BUILD_DIR)/%.io : %.cpp ${CXXFILES} ${HFILES}
	@$(DIRECTORY_GUARD)
	@echo "[C++] [BUILDING $(PROJECT_NAME)] $<"
	@$(CXX) $(CXXHARDFLAGS) -c $< -o $@

$(BUILD_DIR)/%.io : %.c ${CFILES} ${HFILES}
	@$(DIRECTORY_GUARD)
	@echo "[C] [BUILDING $(PROJECT_NAME)] $<"
	@$(CC) $(CHARDFLAGS) -c $< -o $@

clean: 
		rm -rf $(CXXOBJ)
		rm -rf $(COBJ)
