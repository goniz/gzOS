#include <lib/primitives/hashmap.h>
#include <platform/kprintf.h>
#include "cmdline.h"

static const char* _image_name = nullptr;
static char* _cmdline = nullptr;
static int _cmdline_argc = 0;

void cmdline_parse_arguments(int argc, const char** argv)
{
    if (2 != argc) {
        kprintf("[cmdline] not enough arguments.\n");
        return;
    }

    _image_name = argv[0];
    kprintf("[cmdline] %s => %s\n", "image_name", _image_name);

    _cmdline = (char*) argv[1];
    kprintf("[cmdline] cmd => %s\n", _cmdline);

    _cmdline_argc = 0;
    char* pos = _cmdline;
    do {
        char* start = pos;
        char* end = strchr(pos, ' ');
        if (!end) {
            if (0 == strlen(start)) {
                break;
            }

            _cmdline_argc++;
//            kprintf("[cmdline] token: %s\n", start);
            break;
        }

        *end = '\0';

        _cmdline_argc++;
//        kprintf("[cmdline] token: %s\n", start);

        pos = end + 1;

    } while (true);
}

const char* cmdline_get(const char* key) {
    if (nullptr == _cmdline) {
        return nullptr;
    }

    char* pos = _cmdline;
    for (int i = 0; i < _cmdline_argc; i++) {
//        kprintf("[cmdline] get(%s) => token: %s\n", key, pos);

        char* seperator = strchr(pos, '=');
        if (seperator) {
            if (0 == memcmp(pos, key, strlen(key))) {
                return seperator + 1;
            }
        } else {
            if (0 == strcmp(pos, key)) {
                return pos;
            }
        }

        pos += (strlen(pos) + 1);
    }

    return nullptr;
}
