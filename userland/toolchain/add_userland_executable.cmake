
function(add_userland_executable name sources)
    add_executable(${name} ${sources})
    target_link_libraries(${name} c stdc++ supc++ cstubs c)
endfunction()