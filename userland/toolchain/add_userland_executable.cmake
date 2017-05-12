
function(add_userland_executable name sources)
    add_definitions(-mno-gpopt)
    add_executable(${name} ${sources} $<TARGET_OBJECTS:cstubs>)
    target_link_libraries(${name} c stdc++ supc++ m)
    target_include_directories(${name} SYSTEM PRIVATE ${CMAKE_SOURCE_DIR}/toolchain)

    add_custom_command(
            TARGET ${name} POST_BUILD
            COMMAND ${CMAKE_STRIP} -s $<TARGET_FILE:${name}> -o $<TARGET_FILE:${name}>.stripped
    )

endfunction()