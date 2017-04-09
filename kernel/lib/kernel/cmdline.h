#ifndef GZOS_CMDLINE_H
#define GZOS_CMDLINE_H
#ifdef __cplusplus
extern "C" {
#endif

void cmdline_parse_arguments(int argc, const char** argv);
const char* cmdline_get(const char* key);

#ifdef __cplusplus
}
#endif //extern "C"
#endif //GZOS_CMDLINE_H
