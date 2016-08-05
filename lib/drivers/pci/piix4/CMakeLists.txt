cmake_minimum_required( VERSION 2.8 )

project(gzOS_piix4 C CXX ASM)

add_library(piix4 STATIC piix4.cpp piix4.h piix4_regs.h)
