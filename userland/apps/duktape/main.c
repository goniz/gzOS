#include <stdio.h>
#include <unistd.h>
#include "duktape.h"

static const char* source = "function main() { while (true) { print('pid: ' + getpid()); sleep(1); } }; main();";

duk_ret_t duk_current_pid(duk_context* ctx)
{
    duk_push_int(ctx, getpid());
    return 1;
}

duk_ret_t duk_sleep(duk_context* ctx)
{
    sleep(duk_require_uint(ctx, 0));
    return 0;
}

duk_function_list_entry funcs[] = {
        {"getpid", duk_current_pid},
        {"sleep", duk_sleep},
        {NULL, NULL}
};

int main(int argc, char** argv)
{
    puts("Hello Duktape!");
    duk_context* ctx = duk_create_heap_default();
    duk_push_global_object(ctx);
    duk_put_function_list(ctx, -1, funcs);
    duk_pop(ctx);

    duk_eval_string(ctx, source);
    printf("result: %s\n", duk_safe_to_string(ctx, -1));

    duk_destroy_heap(ctx);
    return 0;
}
