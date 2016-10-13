cmake_minimum_required(VERSION 2.8)

project(gzOS_libmalta C CXX ASM)

add_library(malta STATIC
        intr.S
        boot.S
        vectors.S
        interrupts.c
        clock.c
        uart_cbus.c
        init.c
        interrupts.h
        cpu.c
        process.c
        physmem.c
        pmap.c
        tlb.c
        syscall.c
        pci.c
        cache.c
        cacheops.h
        gt64120.h
        gt64120_full.h
        malta.h
        mips.h
        ns16550.h
        pci_registers.h
        script.ld
        tlb_ops.S
)
