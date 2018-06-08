#ifndef GZOS_TCP_STATE_ENUM_H
#define GZOS_TCP_STATE_ENUM_H

#ifdef __cplusplus

enum class TcpStateEnum {
    Closed,
    SynSent,
    SynReceived,
    Established,
    Listening,
    FinSent,
    CloseWait,
    LastAck
};

const char* tcp_state(TcpStateEnum state);


#endif //__cplusplus
#endif //GZOS_TCP_STATE_ENUM_H
