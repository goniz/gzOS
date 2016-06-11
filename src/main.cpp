#include <stdio.h>
#include <memory>
#include <platform/malta/clock.h>

extern "C" char _end;
extern "C" caddr_t _get_stack_pointer(void);

int main(int argc, const char** argv)
{
    printf("Current Stack: %p\n", (void *) _get_stack_pointer());
    printf("Start of heap: %p\n", &_end);

    std::unique_ptr<char[]> buf = std::make_unique<char[]>(1024);
    printf("HELLO %p\n", (void *) buf.get());

    printf("before while\n");
    while (1) { ;
        int c = getchar();
        printf("timer clock %ld\n", clock_get_ms());
        putchar(c + 1);
    }
}

