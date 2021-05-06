PROJECT_NAME := coreutils
DIRECTORY_GUARD=mkdir -p $(@D)

LIBS_DIR := ../../libs/
BUILD_DIR := ../../build/
OUTPUT_PATH := ../../initfs/coreutils/
PROJECT_DIRS := $(LIBS_DIR)
CFILES := $(shell find $(PROJECT_DIRS) -type f -name \*.c)
CXXFILES := $(shell find $(PROJECT_DIRS) -type f -name \*.cpp)
HFILES := $(shell find $(PROJECT_DIRS) -type f -name \*.h)
PROJECT_FILES := $(shell find . -type f -name \*.cpp)
PROJECT_OUTPUT := $(patsubst %.cpp, $(OUTPUT_PATH)/%,$(PROJECT_FILES) )

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

all: $(PROJECT_OUTPUT)
	
clean:
	rm -rf $(PROJECT_OUTPUT)
$(OUTPUT_PATH)/% : %.cpp $(CXXOBJ) ${CXXFILES} ${HFILES} ${PROJECT_FILES}
	@$(DIRECTORY_GUARD)
	@echo "[C++] [BUILDING $(PROJECT_NAME)] $<"
	$(CXX) $(CXXHARDFLAGS) -c $< -o $<.o
	$(LD) -nostdlib -no-pie -z max-page-size=0x1000 -T ../link.ld -o $@ $(COBJ) $(CXXOBJ) $<.o 
	
	
    
$(BUILD_DIR)/%.o : %.cpp ${CXXFILES} ${HFILES}
	@$(DIRECTORY_GUARD)
	@echo "[C++] [BUILDING $(PROJECT_NAME)] $<"
	@$(CXX) $(CXXHARDFLAGS) -c $< -o $@
