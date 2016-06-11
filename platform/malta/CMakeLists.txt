cmake_minimum_required(VERSION 2.8)

project(gzOS_libmalta C ASM)

add_library(malta STATIC intr.S boot.S interrupts.c clock.c uart_cbus.c init.c)
