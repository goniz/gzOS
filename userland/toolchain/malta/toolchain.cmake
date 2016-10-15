# A CMake toolchain file so we can cross-compile for the Rapsberry-Pi bare-metal

# Toolchain can be obtained from "https://community.imgtec.com/developers/mips/tools/codescape-mips-sdk/download-codescape-mips-sdk-essentials/"

# usage
# cmake -DCMAKE_TOOLCHAIN_FILE=../toolchain-arm-none-eabi.cmake ../

# The Generic system _name is used for embedded targets (targets without OS) in
# CMake
set( CMAKE_SYSTEM_NAME          Generic )
set( CMAKE_SYSTEM_PROCESSOR     MIPS    )
set( CMAKE_PLATFORM_NAME		malta CACHE STRING "" )

# The toolchain prefix for all toolchain executables
set( CROSS_COMPILE mips-mti-elf- )

# specify the cross compiler. We force the compiler so that CMake doesn't
# attempt to build a simple test program as this will fail without us using
# the -nostartfiles option on the command line
set(CMAKE_C_COMPILER ${CROSS_COMPILE}gcc)
set(CMAKE_CXX_COMPILER ${CROSS_COMPILE}g++)

# We must set the OBJCOPY setting into cache so that it's available to the
# whole project. Otherwise, this does not get set into the CACHE and therefore
# the build doesn't know what the OBJCOPY filepath is
set( CMAKE_OBJCOPY  ${TC_PATH}${CROSS_COMPILE}objcopy
     CACHE FILEPATH "The toolchain objcopy command " FORCE )

# Set the CMAKE C flags (which should also be used by the assembler!
set( ARCH_FLAGS "-msoft-float -march=mips32r2 -minterlink-mips16 -mno-gpopt -G 0 -mno-abicalls -fno-pic")
set( COMMON_FLAGS "-Os -g0 -s -nostartfiles")
# set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -finstrument-functions" )

set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${ARCH_FLAGS} ${COMMON_FLAGS} -std=gnu11" CACHE STRING "" )
set( CMAKE_ASM_FLAGS "${CMAKE_C_FLAGS} -x assembler-with-cpp" CACHE STRING "" )
set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${ARCH_FLAGS} ${COMMON_FLAGS} -std=c++14 -ffunction-sections -Wl,--gc-sections" CACHE STRING "" )
set( CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-T,${CMAKE_CURRENT_LIST_DIR}/script.ld,--gc-sections" CACHE STRING "" )
