cmake_minimum_required(VERSION 2.8)

project(duktape C CXX ASM)

add_definitions(-DDUK_OPT_FASTINT)
add_userland_executable(duktape
    main.c
    duktape.c
    duktape.h
    duk_config.h
)