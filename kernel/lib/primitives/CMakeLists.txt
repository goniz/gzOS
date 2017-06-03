cmake_minimum_required( VERSION 2.8 )

project(gzOS_libprimitives C CXX ASM)

add_library(
        primitives
        STATIC
        basic_queue.cpp
        basic_queue.h
        InterruptsMutex.h
        SpinlockMutex.h
        SpinlockMutex.cpp
        LockGuard.h
        Suspendable.cpp
        Suspendable.h
        Event.cpp
        Event.h
        hexdump.c
        hexdump.h
        hashmap.c
        hashmap.h
        array.h
        stringutils.cpp
        stringutils.h
        log.h
        Timer.cpp
        Timer.h
        EventStream.h
        AbstractMutex.cpp
        AbstractMutex.h
        SuspendableMutex.cpp
        SuspendableMutex.h
        Mutex.h time.h)
