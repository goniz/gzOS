#ifndef USERLAND_SOCKET_H
#define USERLAND_SOCKET_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Types of sockets.  */
enum __socket_type
{
    SOCK_DGRAM = 1,               /* Connectionless, unreliable datagrams of fixed maximum length.  */
    SOCK_STREAM = 2,              /* Sequenced, reliable, connection-based byte streams.  */
    SOCK_RAW = 3,                 /* Raw protocol interface.  */
};

#define PF_INET         2       /* IP protocol family.  */
#define PF_PACKET       17      /* Packet family.  */

#define AF_INET         PF_INET
#define AF_PACKET       PF_PACKET

/* Standard well-defined IP protocols.  */
enum {
    IPPROTO_IP = 0,               /* Dummy protocol for TCP               */
    IPPROTO_ICMP = 1,             /* Internet Control Message Protocol    */
    IPPROTO_IGMP = 2,             /* Internet Group Management Protocol   */
    IPPROTO_IPIP = 4,             /* IPIP tunnels (older KA9Q tunnels use 94) */
    IPPROTO_TCP = 6,              /* Transmission Control Protocol        */
    IPPROTO_EGP = 8,              /* Exterior Gateway Protocol            */
    IPPROTO_PUP = 12,             /* PUP protocol                         */
    IPPROTO_UDP = 17,             /* User Datagram Protocol               */
    IPPROTO_IDP = 22,             /* XNS IDP protocol                     */
    IPPROTO_TP = 29,              /* SO Transport Protocol Class 4        */
    IPPROTO_DCCP = 33,            /* Datagram Congestion Control Protocol */
    IPPROTO_IPV6 = 41,            /* IPv6-in-IPv4 tunnelling              */
    IPPROTO_RSVP = 46,            /* RSVP Protocol                        */
    IPPROTO_GRE = 47,             /* Cisco GRE tunnels (rfc 1701,1702)    */
    IPPROTO_ESP = 50,             /* Encapsulation Security Payload protocol */
    IPPROTO_AH = 51,              /* Authentication Header protocol       */
    IPPROTO_MTP = 92,             /* Multicast Transport Protocol         */
    IPPROTO_BEETPH = 94,          /* IP option pseudo header for BEET     */
    IPPROTO_ENCAP = 98,           /* Encapsulation Header                 */
    IPPROTO_PIM = 103,            /* Protocol Independent Multicast       */
    IPPROTO_COMP = 108,           /* Compression Header Protocol          */
    IPPROTO_SCTP = 132,           /* Stream Control Transport Protocol    */
    IPPROTO_UDPLITE = 136,        /* UDP-Lite (RFC 3828)                  */
    IPPROTO_RAW = 255,            /* Raw IP packets                       */
    IPPROTO_MAX
};

typedef struct {
    uint32_t address;
    uint16_t port;
} sockaddr_t;

/* Create a new socket of type TYPE in domain DOMAIN, using
   protocol PROTOCOL.  If PROTOCOL is zero, one is chosen automatically.
   Returns a file descriptor for the new socket, or -1 for errors.  */
int socket(int __domain, int __type, int __protocol);

/* Give the socket FD the local address ADDR (which is LEN bytes long).  */
int bind(int __fd, const sockaddr_t*__addr, size_t __len);

/* Send N bytes of BUF to socket FD.  Returns the number sent or -1.

   This function is a cancellation point and therefore not marked with
   __THROW.  */
int send(int __fd, const void *__buf, size_t __n, int __flags);

/* Send N bytes of BUF on socket FD to peer at address ADDR (which is
   ADDR_LEN bytes long).  Returns the number sent, or -1 for errors.

   This function is a cancellation point and therefore not marked with
   __THROW.  */
int sendto(int __fd, const void *__buf, size_t __n,
           int __flags, const sockaddr_t* __addr,
           size_t __addr_len);


/* Read N bytes into BUF from socket FD.
   Returns the number read or -1 for errors.

   This function is a cancellation point and therefore not marked with
   __THROW.  */
int recv(int __fd, void *__buf, size_t __n, int __flags);

/* Read N bytes into BUF through socket FD.
   If ADDR is not NULL, fill in *ADDR_LEN bytes of it with tha address of
   the sender, and store the actual size of the address in *ADDR_LEN.
   Returns the number of bytes read or -1 for errors.

   This function is a cancellation point and therefore not marked with
   __THROW.  */
int recvfrom(int __fd, void * __buf, size_t __n,
             int __flags, sockaddr_t* __addr,
             size_t * __addr_len);


#ifdef __cplusplus
}
#endif //extern "C"
#endif //USERLAND_SOCKET_H
