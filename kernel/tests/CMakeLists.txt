cmake_minimum_required( VERSION 2.8 )

# Mark the language as C so that CMake doesn't try to test the C++
# cross-compiler's ability to compile a simple program because that will fail
project(tests C CXX ASM)

add_executable(tests main.cpp)

target_link_libraries(tests -Wl,--whole-archive ${CMAKE_PLATFORM_NAME} platform stubs -Wl,--no-whole-archive)
target_link_libraries(tests -Wl,--whole-archive i8259A piix4 pcnet e1000 qemu_shmem pflash fat32 procfs -Wl,--no-whole-archive)
target_link_libraries(tests -Wl,--whole-archive mm network malloc kernel primitives syscall -Wl,--no-whole-archive)
target_link_libraries(tests c stdc++ supc++)
set_target_properties(tests PROPERTIES LINK_FLAGS "-Wl,-Map,${CMAKE_BINARY_DIR}/tests.map")

add_custom_command(
    TARGET tests POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} tests${CMAKE_EXECUTABLE_SUFFIX} -O binary ./tests.img
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Convert the ELF output file to a binary image" )
