
function(add_userland_executable name sources)
    add_definitions(-mno-gpopt)
    add_executable(${name} ${sources})
    target_link_libraries(${name} c stdc++ supc++ cstubs c m)
    target_include_directories(${name} SYSTEM PRIVATE ${CMAKE_SOURCE_DIR}/toolchain)
endfunction()