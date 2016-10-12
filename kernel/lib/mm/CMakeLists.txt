cmake_minimum_required( VERSION 2.8 )

project(gzOS_libmm C CXX ASM)

add_library(mm STATIC
        physmem.c physmem.h
        pmap.h vm.h
        vm_map.c vm_map.h
        vm_object.c vm_object.h
        vm_pager.c vm_pager.h
)
