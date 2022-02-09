#ifndef MULTIPLAYER_H
#define MULTIPLAYER_H

#include <stddef.h>

#define PACK_DECIMAL_SIZE 11
#define PACK_LONG_DECIMAL_SIZE 20
#define PACK_BOOL_SIZE 1

#define PORT "1234"

typedef int Player;

typedef enum PacketType { 
    MULTI_START,
    MULTI_PAUSE,
    MULTI_UPDATE
} PacketType;

int pack(char *str, const char *const fmt, ...);
int unpack(char *str, const char *const fmt, ...);
size_t fmt_length(const char *const fmt);
int sendall(const int sockfd, const char *const buf, const int len);
int recvall(const int sockfd, char *const buf, const int len);
void send_packet_type(const int sockfd, const PacketType type);
PacketType recv_packet_type(const int sockfd);
int get_client_socket(const char *const ip, const char *const port);

#endif
