
function(add_userland_executable name sources)
    add_definitions(-mno-gpopt)
    add_executable(${name} ${sources})
    target_link_libraries(${name} c stdc++ supc++ cstubs c m)
    target_include_directories(${name} SYSTEM PRIVATE ${CMAKE_SOURCE_DIR}/toolchain)

    add_custom_command(
            TARGET ${name} POST_BUILD
            COMMAND ${CMAKE_STRIP} -s $<TARGET_FILE:${name}> -o $<TARGET_FILE:${name}>.stripped
    )

    add_custom_target(${name}-run)
    add_custom_command(
            TARGET ${name}-run
            COMMAND python2 ${CMAKE_SOURCE_DIR}/../sendfile.py $<TARGET_FILE:${name}>.stripped
            COMMENT "Sending elf: ${name}"
    )
endfunction()