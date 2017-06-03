#ifndef GZOS_MUTEX_H
#define GZOS_MUTEX_H
#ifdef __cplusplus
extern "C" {
#endif

struct Mutex {
    virtual ~Mutex() = default;

    virtual void lock(void) = 0;
    virtual void unlock(void) = 0;
};

#ifdef __cplusplus
}
#endif //extern "C"
#endif //GZOS_MUTEX_H
