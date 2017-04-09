#ifndef USERLAND_KILL_H
#define USERLAND_KILL_H
#ifdef __cplusplus
extern "C" {
#endif

/*
 kill
 Send a signal. Minimal implementation:
 */
int kill(int pid, int sig);

#ifdef __cplusplus
}
#endif //extern "C"
#endif //USERLAND_KILL_H
