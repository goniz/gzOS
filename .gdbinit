target remote localhost:1234
define xxd
dump binary memory /tmp/dump.bin $arg0 $arg0+$arg1
shell xxd /tmp/dump.bin
end
b panic
b kernel_oops
b vm_do_segfault
skip malloc
skip printf
skip kprintf
skip vsprintf
skip memcpy
skip memcmp
