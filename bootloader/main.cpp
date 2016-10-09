//
// Created by gz on 3/11/16.
//

#include <stdio.h>

extern "C"
int kernel_main(void* argument)
{
    printf("Hello!\n");
    return 0;
}
