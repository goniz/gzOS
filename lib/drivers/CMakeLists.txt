cmake_minimum_required( VERSION 2.8 )

project(gzOS_drivers C CXX ASM)

add_subdirectory(pci/piix4)
add_subdirectory(pci/pcnet)
add_subdirectory(pci/e1000)
add_subdirectory(pci/virtio_shmem)
add_subdirectory(i8259A)