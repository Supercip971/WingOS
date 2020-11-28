PROJECT_NAME := graphic_service
PROJECT_DIRS := . ../../usr_lib
CFILES := $(shell find $(PROJECT_DIRS) -type f -name \*.cpp)
HFILES := $(shell find $(PROJECT_DIRS) -type f -name \*.h)

OBJ := ${CFILES:.cpp=.o}

CC = ../../cross_compiler/bin/x86_64-pc-elf-g++
LD = ../../cross_compiler/bin/x86_64-pc-elf-ld
CHARDFLAGS := $(CFLAGS)               \
        -DBUILD_TIME='"$(BUILD_TIME)"' \
        -std=c++20                     \
        -g \
        -masm=intel                    \
        -fno-pic                       \
        -no-pie \
		-m64 \
        -O2 \
				-msse \
        -mno-80387                     \
        -mno-red-zone                  \
        -fno-rtti \
        -fno-exceptions \
		-ffreestanding                 \
        -fno-stack-protector           \
        -fno-omit-frame-pointer        \
        -I../../usr_lib -I.\

all: ${OBJ}
	@$(LD) -nostdlib -no-pie -z max-page-size=0x1000 -T ../link.ld -o $(PROJECT_NAME).exe $(OBJ)
	@$(LD) -nostdlib -no-pie -z max-page-size=0x1000 -T ../link.ld -o ../../init_fs/$(PROJECT_NAME).exe $(OBJ)


%.o : %.cpp ${CFILES} ${HFILES}
	@echo "[C++] [BUILDING $(PROJECT_NAME)] $<"
	@$(CC) $(CHARDFLAGS) -c $< -o $@


