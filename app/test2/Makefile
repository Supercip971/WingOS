PROJECT_NAME := test2
DIRECTORY_GUARD=mkdir -p $(@D)

LIBS_DIR := ../../libs/
BUILD_DIR := ../../build/
PROJECT_DIRS := . $(LIBS_DIR)
CFILES := $(shell find $(PROJECT_DIRS) -type f -name \*.c)
CXXFILES := $(shell find $(PROJECT_DIRS) -type f -name \*.cpp)
HFILES := $(shell find $(PROJECT_DIRS) -type f -name \*.h)

CXXOBJ := $(patsubst %.cpp,$(BUILD_DIR)/%.o,$(CXXFILES) )
COBJ := $(patsubst %.c,$(BUILD_DIR)/%.o, $(CFILES))
CXX = ../../cross_compiler/bin/x86_64-pc-wingos-g++
CC = ../../cross_compiler/bin/x86_64-pc-wingos-gcc
LD = ../../cross_compiler/bin/x86_64-pc-wingos-ld
CXXHARDFLAGS := $(CFLAGS)               \
        -DBUILD_TIME='"$(BUILD_TIME)"' \
        -std=c++20                     \
        -g \
        -masm=intel                    \
        -fno-pic                       \
        -no-pie \
        -m64 \
        -O1 \
        -Werror \
        -msse \
        -mno-avx \
        -mno-avx2 \
        -mno-avx512f \
        -mno-80387                     \
        -mno-red-zone                  \
        -fno-rtti \
        -fno-exceptions \
		-ffreestanding                 \
        -fno-stack-protector           \
        -fno-omit-frame-pointer        \
        -I$(LIBS_DIR) -I$(LIBS_DIR)/libc -I$(LIBS_DIR)/utils \

CHARDFLAGS := $(CFLAGS)               \
        -DBUILD_TIME='"$(BUILD_TIME)"' \
        -std=c11                     \
        -g \
        -masm=intel                    \
        -fno-pic                       \
        -no-pie \
        -m64 \
        -O1 \
        -Werror \
        -msse \
        -mno-avx \
        -mno-avx2 \
        -mno-avx512f \
        -mno-80387                     \
        -mno-red-zone                  \
        -fno-rtti \
        -fno-exceptions \
		-ffreestanding                 \
        -fno-stack-protector           \
        -fno-omit-frame-pointer        \
        -I$(LIBS_DIR) -I$(LIBS_DIR)/libc -I$(LIBS_DIR)/utils \

all: $(CXXOBJ) $(COBJ)
	@$(LD) -nostdlib -no-pie -z max-page-size=0x1000 -T ../link.ld -o $(PROJECT_NAME).exe $(COBJ) $(CXXOBJ)
	@$(LD) -nostdlib -no-pie -z max-page-size=0x1000 -T ../link.ld -o ../../init_fs/$(PROJECT_NAME).exe $(COBJ) $(CXXOBJ)


$(BUILD_DIR)/%.o : %.cpp ${CXXFILES} ${HFILES}
	@$(DIRECTORY_GUARD)
	@echo "[C++] [BUILDING $(PROJECT_NAME)] $<"
	@$(CXX) $(CXXHARDFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o : %.c ${CFILES} ${HFILES}
	@$(DIRECTORY_GUARD)
	@echo "[C] [BUILDING $(PROJECT_NAME)] $<"
	@$(CC) $(CHARDFLAGS) -c $< -o $@
