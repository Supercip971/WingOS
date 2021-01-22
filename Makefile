DIRECTORY_GUARD=mkdir -p $(@D)

ARCH := x86_64
END_PATH := kernel/generic kernel/arch/$(ARCH)
# kernel files
CFILES    := $(shell find $(END_PATH) -type f -name '*.cpp')
HFILES    := $(shell find $(END_PATH) -type f -name '*.h')
ASMFILES  := $(shell find $(END_PATH) -type f -name '*.asm')
OBJFILES := $(patsubst %.cpp,%.o,$(CFILES))
ASMOBJFILES := $(patsubst %.asm,%.o,$(ASMFILES))
DPEND_FILES := $(patsubst %.cpp,%.d,$(CFILES))
LINK_PATH := ./kernel/arch/$(ARCH)/linker.ld

# user_lib code files
USRCFILES    := $(shell find usr_lib/ -type f -name '*.cpp')
USRHFILES    := $(shell find usr_lib/ -type f -name '*.h')
# user app code files
USRAPPCFILES := $(shell find app/ -type f -name '*.cpp')
USRAPPHFILES := $(shell find app/ -type f -name '*.h')


CC         = ./cross_compiler/bin/x86_64-pc-elf-g++
LD         = ./cross_compiler/bin/x86_64-pc-elf-ld
ECHFS_PATH = ./echfs/echfs-utils

OBJ := $(shell find $(END_PATH) -type f -name '*.o')

APP_FS_MAKEFILE_FLAGS 	= all -j$(nproc)
APP_FS_CHANGE 			= ./usr_lib/ ./app/
APP_FILE_CHANGE 		= $(shell find $(APP_FS_CHANGE) -type f -name '*.cpp')

KERNEL_HDD = ./build/disk.hdd
KERNEL_ELF = kernel.elf

CHARDFLAGS := $(CFLAGS)               \
        -DBUILD_TIME='"$(BUILD_TIME)"' \
        -std=c++20                     \
        -g \
        -masm=intel                    \
        -fno-pic                       \
        -no-pie \
        -m64 \
		-Wall \
		-MD \
		-MMD \
		-Werror \
        -O2 \
        -mcmodel=kernel \
        -mno-80387                     \
        -mno-red-zone                  \
        -fno-rtti \
        -fno-exceptions \
		-ffreestanding                 \
        -fno-stack-protector           \
        -fno-omit-frame-pointer        \
		-fno-isolate-erroneous-paths-attribute \
		-fno-delete-null-pointer-checks \
		-I./kernel/generic                        \
		-I./kernel/arch/$(ARCH) \

LDHARDFLAGS := $(LDFLAGS)        \
        -nostdlib                 \
        -no-pie                   \
        -z max-page-size=0x1000   \
        -T $(LINK_PATH)

.DEFAULT_GOAL = $(KERNEL_HDD)

.PHONY:setup_echfs_utils
setup_echfs_utils:
	@make -C echfs/ clean -j$(nproc)
	@make -C echfs/ all -j$(nproc)

.PHONY:setup_toolchain
setup_toolchain:
	@bash ./make_cross_compiler.sh

.PHONY:setup_limine
setup_limine:
	-rm ./limine/limine-install
	@make -C limine/ limine-install -j$(nproc)

.PHONY:first_setup
first_setup: 
	@make setup_toolchain -j$(nproc)
	@make setup_echfs_utils -j$(nproc)
	@make setup_limine -j$(nproc)

.PHONY:bochs
bochs: $(KERNEL_HDD)
	-rm disk.img
	@bximage -q -mode=convert -imgmode=flat $(KERNEL_HDD) disk.img
	@bochs

.PHONY:disk
disk: $(KERNEL_HDD)

.PHONY:run
run: $(KERNEL_HDD)
	qemu-system-x86_64 -m 4G -s -device pvpanic -smp 6 -serial stdio -enable-kvm --no-shutdown --no-reboot -d int -d guest_errors -hda $(KERNEL_HDD) \
		-nic user,model=e1000 -M q35 

.PHONY:runvbox
runvbox: $(KERNEL_HDD)
	@VBoxManage -q startvm --putenv VBOX_GUI_DBG_ENABLED=true wingOS64
	@nc localhost 1234

.PHONY:format
format:
	@clang-format -i --style=file $(CFILES) $(HFILES)
	@clang-format -i --style=file $(USRCFILES) $(USRHFILES)
	@clang-format -i --style=file $(USRAPPCFILES) $(USRAPPHFILES)

foreachramfs: 
	@for f in $(shell find init_fs/ -maxdepth 64 -type f); do $(ECHFS_PATH) -m -p0 $(KERNEL_HDD) import $${f} $${f}; done

.PHONY:app
app: $(APP_FILE_CHANGE)
	@make -C ./app/test $(APP_FS_MAKEFILE_FLAGS)	
	@make -C ./app/test2 $(APP_FS_MAKEFILE_FLAGS)	
	@make -C ./app/graphic_service $(APP_FS_MAKEFILE_FLAGS)	
	@make -C ./app/memory_service $(APP_FS_MAKEFILE_FLAGS)	
	@make -C ./app/console_service $(APP_FS_MAKEFILE_FLAGS)	
	@make -C ./app/background $(APP_FS_MAKEFILE_FLAGS)	
	@make -C ./app/wstart $(APP_FS_MAKEFILE_FLAGS)	
	@make -C ./app/shell $(APP_FS_MAKEFILE_FLAGS)	

.PHONY:super
super:
	@make app -j12
	@make format
	@make -j12

	@objdump kernel.elf -f -s -d --source > kernel.map
	@make run

.PHONY:check
check:
	@make clean
	@make $(KERNEL_ELF) -j12
	@make app -j12

-include $(DPEND_FILES)
%.o: %.cpp 
	@echo "[KERNEL $(ARCH)] (cpp) $<"
	@$(CC) $(CHARDFLAGS) -c $< -o $@

%.o: %.asm
	@echo "[KERNEL $(ARCH)] (nasm) $<"
	@nasm $< -o $@ -felf64 -F dwarf -g -w+all -Werror


.PHONY:$(KERNEL_ELF)
$(KERNEL_ELF): $(OBJFILES) $(ASMOBJFILES) $(LINK_PATH)
	@ld $(LDHARDFLAGS) $(OBJFILES) $(ASMOBJFILES) -o $@

$(KERNEL_HDD): $(KERNEL_ELF) 
	-rm -rf $(KERNEL_HDD)
	-mkdir build
	bash ./make_disk.sh
	limine/limine-install limine/limine.bin $(KERNEL_HDD)

.PHONY:clean
clean:
	-rm -f $(DPEND_FILES)
	-rm -f app/**.o
	-rm -f usr_lib/**.o
	-rm -f $(KERNEL_HDD) $(KERNEL_ELF) $(OBJ)

.PHONY:all
all:
	@make -C . super
