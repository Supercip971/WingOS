ARCH := x86_64
END_PATH := kernel/generic kernel/arch/$(ARCH)



CFILES    := $(shell find $(END_PATH) -type f -name '*.cpp')
HFILES    := $(shell find $(END_PATH) -type f -name '*.h')
USRCFILES    := $(shell find usr_lib/ -type f -name '*.cpp')
USRHFILES    := $(shell find usr_lib/ -type f -name '*.h')
USRAPPCFILES := $(shell find app/ -type f -name '*.cpp')
USRAPPHFILES := $(shell find app/ -type f -name '*.h')
CC         = ./cross_compiler/bin/x86_64-pc-elf-g++
LD         = ./cross_compiler/bin/x86_64-pc-elf-ld
OBJ := $(shell find $(END_PATH) -type f -name '*.o')

KERNEL_HDD = ./build/disk.hdd
KERNEL_RAMDISK = ./build/ramdisk.hdd

APP_FS_CHANGE = ./usr_lib/ ./app/
APP_FILE_CHANGE := $(shell find $(APP_FS_CHANGE) -type f -name '*.cpp')
KERNEL_ELF = kernel.elf
ASMFILES := $(shell find $(END_PATH) -type f -name '*.asm')
LINK_PATH := ./kernel/arch/$(ARCH)/linker.ld
OBJFILES := $(patsubst %.cpp,%.o,$(CFILES))
ASMOBJFILES := $(patsubst %.asm,%.o,$(ASMFILES))
CHARDFLAGS := $(CFLAGS)               \
        -DBUILD_TIME='"$(BUILD_TIME)"' \
        -std=c++20                     \
        -g \
        -masm=intel                    \
        -fno-pic                       \
        -no-pie \
        -m64 \
		-Wall \
		-Werror \
        -O3 \
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
ECHFS_PATH := ./echfs/echfs-utils
.PHONY: clean boch disk run runvbox format app super all check
.DEFAULT_GOAL = $(KERNEL_HDD)
setup_echfs_utils:
	@make -C echfs/ clean -j$(nproc)
	@make -C echfs/ all -j$(nproc)

setup_toolchain:
	@bash ./make_cross_compiler.sh
setup_limine:
	-rm ./limine/limine-install
	@make -C limine/ limine-install -j$(nproc)
first_setup: 
	@make setup_toolchain -j$(nproc)
	@make setup_echfs_utils -j$(nproc)
	@make setup_limine -j$(nproc)
boch:
	-rm disk.img
	@bximage -q -mode=convert -imgmode=flat build/disk.hdd disk.img
	@bochs

disk:
	@rm -rf $(KERNEL_HDD)
	@make -C . $(KERNEL_HDD)
run: $(KERNEL_HDD)
	qemu-system-x86_64 -m 4G -s -device pvpanic -smp 6 -serial stdio -enable-kvm --no-shutdown --no-reboot -d int -d guest_errors -hda $(KERNEL_HDD) \
		-nic user,model=e1000 -M q35 
runvbox: $(KERNEL_HDD)
	@VBoxManage -q startvm --putenv VBOX_GUI_DBG_ENABLED=true wingOS64
	@nc localhost 1234
format:
	@clang-format -i --style=file $(CFILES) $(HFILES)
	@clang-format -i --style=file $(USRCFILES) $(USRHFILES)
	@clang-format -i --style=file $(USRAPPCFILES) $(USRAPPHFILES)
foreachramfs: 
	@for f in $(shell find init_fs/ -maxdepth 64 -type f); do $(ECHFS_PATH) -m -p0 $(KERNEL_HDD) import $${f} $${f}; done

app: $(APP_FILE_CHANGE)
	@make -C ./app/test all -j12	
	@make -C ./app/test2 all -j12
	@make -C ./app/graphic_service all -j12
	@make -C ./app/memory_service all -j12
	@make -C ./app/console_service all -j12
	@make -C ./app/background all -j12
	@make -C ./app/wstart all -j12

super:
	@make clean 
	@make app -j12
	-killall -9 VirtualBoxVM
	-killall -9 qemu-system-x86_64
	@make format
	@make -j12

	@objdump kernel.elf -f -s -d --source > kernel.map
	@make run
check:
	@make clean
	@make $(KERNEL_ELF) -j12
	@make app -j12

%.o: %.cpp %.h
	@echo "cpp [BUILD] $< $(CHARDFLAGS)"
	@$(CC) $(CHARDFLAGS) -c $< -o $@

%.o: %.asm
	@echo "nasm [BUILD] $<"
	@nasm $< -o $@ -felf64 -F dwarf -g -w+all -Werror
$(KERNEL_ELF): $(OBJFILES) $(ASMOBJFILES)
	@ld $(LDHARDFLAGS) $(OBJFILES) $(ASMOBJFILES) -o $@

$(KERNEL_HDD): $(KERNEL_ELF)
	-rm -rf $(KERNEL_HDD)
	-mkdir build
	@dd if=/dev/zero bs=8M count=0 seek=64 of=$(KERNEL_HDD)
	@parted -s $(KERNEL_HDD) mklabel msdos
	@parted -s $(KERNEL_HDD) mkpart primary 1 100%
	@$(ECHFS_PATH) -m -p0 $(KERNEL_HDD) format 4096
	@$(ECHFS_PATH) -m -p0 $(KERNEL_HDD) import $(KERNEL_ELF) $(KERNEL_ELF)
	@make -C . foreachramfs	
	@$(ECHFS_PATH) -m -p0 $(KERNEL_HDD) import limine.cfg limine.cfg
	limine/limine-install limine/limine.bin $(KERNEL_HDD)

clean:
	-rm -f app/**.o
	-rm -f usr_lib/**.o
	-rm -f $(KERNEL_HDD) $(KERNEL_ELF) $(OBJ)
all:
	@make -C . super
