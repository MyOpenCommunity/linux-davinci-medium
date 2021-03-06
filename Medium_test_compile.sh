#!/bin/bash

> Test_Compile.log

export CROSS_COMPILE=/devel/devel/PLATFORMS/NEXTGEN/arm-2009q1/bin/arm-none-linux-gnueabi-
export ARCH=arm

make mrproper
make basi_defconfig
make -j16 uImage && echo "Basi OK" >> Test_Compile.log 
make mrproper
make dingo_defconfig
make -j16 uImage && echo "Dingo OK" >> Test_Compile.log 
make mrproper
make amico-i_defconfig
make -j16 uImage && echo "Amico-i OK" >> Test_Compile.log 
make mrproper
make amico-e_defconfig
make -j16 uImage && echo "Amico-e OK" >> Test_Compile.log
make mrproper
make amico-s_defconfig
make -j16 uImage && echo "Amico-s OK" >> Test_Compile.log
make mrproper
make amico-p_defconfig
make -j16 uImage && echo "Amico-p OK" >> Test_Compile.log
make mrproper
make lago_defconfig
make -j16 uImage && echo "Lago OK" >> Test_Compile.log 
make mrproper
make jumbo-i_defconfig
make -j16 uImage && echo "Jumbo-i OK" >> Test_Compile.log 
make mrproper
make owl_defconfig
make -j16 uImage && echo "Owl OK" >> Test_Compile.log 
make mrproper
make jumbo-d_defconfig
make -j16 uImage && echo "Jumbo-d OK" >> Test_Compile.log 
make mrproper
make seah_defconfig
make -j16 uImage && echo "Seah OK" >> Test_Compile.log
make gekko_defconfig
make -j16 uImage && echo "Gekko OK" >> Test_Compile.log
