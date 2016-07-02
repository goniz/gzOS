cmake_minimum_required(VERSION 2.8)

project(gzOS_libmalta C CXX ASM)

add_library(malta STATIC intr.S boot.S interrupts.c clock.c uart_cbus.c init.c interrupts.h cpu.c process.c pm.c pmap.c tlb.c tlb-mips.S syscall.c pci.c)
