#ifndef GZOS_TCP_SESSIONS_H
#define GZOS_TCP_SESSIONS_H

#ifdef __cplusplus

#include "lib/network/socket.h"
#include "lib/network/tcp/tcp_socket.h"

void registerTcpDescriptor(TcpFileDescriptor* fileDescriptor);
void removeTcpDescriptor(TcpFileDescriptor* fileDescriptor);
TcpFileDescriptor* getTcpDescriptorByFourTuple(const SocketAddress& src, const SocketAddress& dst);
uint16_t allocatePort(uint16_t port);
bool releasePort(uint16_t port);

#endif //__cplusplus
#endif //GZOS_TCP_SESSIONS_H
