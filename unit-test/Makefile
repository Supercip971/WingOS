DIR_GUARD=mkdir -p $(@D)
BUILD_PATH := ./build/
PROJECT_DIR := ./src/ ../libs/utils/ ../libs/libc/
PLUG_FILE_OBJ   := ./plug/unit-test-plug.to
PLUG_FILE_CPP   := ./plug/unit-test-plug.cpp
CFILES := $(shell find $(PROJECT_DIR) -type f -name *.cpp)
HFILES := $(shell find $(PROJECT_DIR) -type f -name *.h)

SANITIZER := -fsanitize=address -fsanitize=leak -fsanitize=undefined

OBJFILES := $(patsubst %.cpp, %.to, $(CFILES))
OUTPUT := $(BUILD_PATH)test

LIB := 


GCC_FLAGS := \
	-g \
	-O0 \
	-m64 \
	-Wall \
	-fno-rtti \
	-Werror \
  -pthread \
	-I./src/ \
	-I../libs/ \
	-std=c++14 \
	-DUNIT_TEST \
	-DCUSTOM_PLUG \
	--freestanding \
	-I../libs/libc \

%.to: %.cpp $(CFILES) $(HFILES)
	@$(DIR_GUARD)
	@echo " [ CXX ] $<"
	@g++ -c $(GCC_FLAGS) $(SANITIZER) -nostdlib -no-pie -o $@ $<

$(PLUG_FILE_OBJ): $(PLUG_FILE_CPP)
	@$(DIR_GUARD)
	@echo " [ PLUG ] $<"
	@g++ -c  -I../libs/libc/plug/ -std=c++14 -m64 -O0 -g $(LIB) $(SANITIZER) -no-pie -o $@ $<

$(OUTPUT): $(OBJFILES) $(PLUG_FILE_OBJ)
	@$(DIR_GUARD)
	@echo " [ unit-test ] $(OUTPUT) "
	@g++ $(OBJFILES) $(PLUG_FILE_OBJ) $(LIB) $(SANITIZER) --freestanding -no-pie  -m64 -g -pthread -o $(OUTPUT)


clean:
	rm $(OBJFILES)
	rm $(OUTPUT) $(PLUG_FILE_OBJ)


all: $(OUTPUT)

run:
	make -C . all
	$(OUTPUT)
	true make -C . clean
format:
	clang-format -i --style=file $(CFILES) $(HFILES) $(PLUG_FILE_OBJ)
                          
