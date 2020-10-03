## before
you can't build wingOS on windows or in WSL


# build guide 
how to build this os ?

## set up
you have to install these packages : 

```build-essential bison flex libgmp3-dev libmpc-dev libmpfr-dev texinfo```

before making the toolchain

## making the toolchain
then run the file make\_cross\_compiler.sh

## making wingOS

you have to run make to build wingOS
there are some option : 



- make clean

to clean



- make format 

to run clang format



- make super

to clean, rebuild and run everything



- make app

to make wingOS app



- make runvbox

for running virtual box (you have to create a virtual machine named : wingOS64



- make run

for running qemu



- make boch

for running boch



- make disk 

for making the disk


