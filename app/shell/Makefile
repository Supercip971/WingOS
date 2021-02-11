PROJECT_NAME := shell
DIRECTORY_GUARD=mkdir -p $(@D)

LIBS_DIR := ../../libs/
BUILD_DIR := ../../build/
PROJECT_DIRS := . $(LIBS_DIR)
CFILES := $(shell find $(PROJECT_DIRS) -type f -name \*.cpp)
HFILES := $(shell find $(PROJECT_DIRS) -type f -name \*.h)

OBJ := $(patsubst %.cpp,$(BUILD_DIR)/%.o,$(CFILES))
CC = ../../cross_compiler/bin/x86_64-pc-wingos-g++
LD = ../../cross_compiler/bin/x86_64-pc-wingos-ld
CHARDFLAGS := $(CFLAGS)               \
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

all: ${OBJ}
	@$(LD) -nostdlib -no-pie -z max-page-size=0x1000 -T ../link.ld -o $(PROJECT_NAME).exe $(OBJ)
	@$(LD) -nostdlib -no-pie -z max-page-size=0x1000 -T ../link.ld -o ../../init_fs/$(PROJECT_NAME).exe $(OBJ)


$(BUILD_DIR)/%.o : %.cpp ${CFILES} ${HFILES}
	$(DIRECTORY_GUARD)
	@echo "[C++] [BUILDING $(PROJECT_NAME)] $<"
	@$(CC) $(CHARDFLAGS) -c $< -o $@
