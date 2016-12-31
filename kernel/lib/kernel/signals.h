#ifndef GZOS_SIGNALS_H
#define GZOS_SIGNALS_H

enum Signal
{
    SIG_KILL,
    SIG_STOP,
    SIG_CONT,
    SIG_ABORT = 6,
    SIG_NONE = -1
};

#endif //GZOS_SIGNALS_H
