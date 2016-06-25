cmake_minimum_required( VERSION 2.8 )

# Mark the language as C so that CMake doesn't try to test the C++
# cross-compiler's ability to compile a simple program because that will fail
project(gzOS C CXX ASM)

add_executable(gzOS main.cpp)

target_link_libraries(gzOS -Wl,--whole-archive ${CMAKE_PLATFORM_NAME} platform stubs -Wl,--no-whole-archive)
target_link_libraries(gzOS malloc scheduler primitives)
target_link_libraries(gzOS c stdc++ supc++)
set_target_properties(gzOS PROPERTIES LINK_FLAGS "-Wl,-Map,${CMAKE_BINARY_DIR}/gzOS.map")

#add_custom_command(
#        TARGET gzOS
#        POST_BUILD
#        COMMAND ln ./src/gzOS${CMAKE_EXECUTABLE_SUFFIX} gzOS.elf
#        WORKING_DIRECTORY ${CMAKE_BINARY_DIR})

add_custom_command(
    TARGET gzOS POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} gzOS${CMAKE_EXECUTABLE_SUFFIX} -O binary ./gzOS.img
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Convert the ELF output file to a binary image" )
