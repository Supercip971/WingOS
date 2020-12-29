# WingOS Build Guide !

Before anything you need to use linux (wsl 2 may work)

## SETUP

before everything you have to install :
`
qemu make build-essential bison flex libgmp3-dev libmpc-dev libmpfr-dev texinfo nasm mtools wget unzip fuse libfuse-dev uuid-dev gcc binutils parted
`

you can install everything in debian distribution with this command :
```bash
sudo apt install  make build-essential bison flex libgmp3-dev libmpc-dev libmpfr-dev texinfo nasmqemu-system-x86 mtools wget unzip fuse libfuse-dev uuid-dev gcc binutils parted
```

## CLONING

when you clone the project you have to clone submodules (echfs and limine)
```bash
git clone --recursive https://github.com/Supercip971/WingOS_x64.git
```

## SETUP #2

to setup everything you can just run the command 
```bash
make first_setup -j$(nproc)
```

*WARNING:* this step will take a lot of time

*WARNING:* at one moment we need sudo for installing echfs 
```
sudo make -C echfs/ install -j$(nproc) 
```

## BUILD & RUN

### BUILD

you can just do 
```bash
make disk
```

### RUN

for running with qemu you just have to run 
```bash
make run 
```