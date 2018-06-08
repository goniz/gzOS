#include <lib/network/tcp/states/TcpStateEnum.h>

static const char* _states[] = {
        "Closed",
        "Syn-Sent",
        "Syn-Received",
        "Established",
        "Listen",
        "Fin-Sent",
        "Close-Wait",
        "Last-Ack"
};

const char* tcp_state(TcpStateEnum state) {
    return _states[int(state)];
}
