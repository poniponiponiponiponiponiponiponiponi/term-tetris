#include <arpa/inet.h>
#include <netdb.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <unistd.h>

#include "debug.h"
#include "multiplayer.h"

/* In the pack and friends functions for the fmt parameter you can provide
 * letters responding for particular variable types. 'd' is int,
 * 'l' is long long, a number followed by 's' is a char[number] array,
 * 'b' is bool.
 * Also in those function there's some fuckery with memcpy. It's not the
 * fastest way to do it for sure however this is the simplest thing that I
 * come up with to get rid off the null byte at the end of sprintf functions.
 */

int pack(char *str, const char *const fmt, ...) {
    va_list argp;
    va_start(argp, fmt);

    char to[PACK_LONG_DECIMAL_SIZE+1];

    int written = 0;
    
    int number = 0;
    for (size_t i = 0; fmt[i] != '\0'; i++) {
        int digit = -1;
        memset(to, '\0', sizeof to * sizeof to[0]);

        switch (fmt[i]) {
        case 's':
            memcpy(str, va_arg(argp, char *), number);
            str += number;
            written += number;
            break;
        case 'd':
            snprintf(to, PACK_DECIMAL_SIZE+1, "%d", va_arg(argp, int));
            memcpy(str, to, PACK_DECIMAL_SIZE);
            str += PACK_DECIMAL_SIZE;
            written += PACK_DECIMAL_SIZE;
            break;
        case 'l':
            snprintf(to, PACK_LONG_DECIMAL_SIZE+1, "%lld",
                    va_arg(argp, long long));
            memcpy(str, to, PACK_LONG_DECIMAL_SIZE);
            str += PACK_LONG_DECIMAL_SIZE;
            written += PACK_LONG_DECIMAL_SIZE;
            break;
        case 'b':
            snprintf(to, PACK_BOOL_SIZE+1, "%c", va_arg(argp, int));
            memcpy(str, to, PACK_BOOL_SIZE);
            str += PACK_BOOL_SIZE;
            written += PACK_BOOL_SIZE;
            break;
        case '1': case '2': case '3': case '4': case '5':
        case '6': case '7': case '8': case '9': case '0':
            digit = fmt[i] - '0';
            break;
        default:
            fprintf(stderr, "weird pack argument\n");
            return written;
        }

        number = number * 10 + digit;
        if (digit == -1)
            number = 0;
    }

    va_end(argp);

    return written;
}

int unpack(char *str, const char *const fmt, ...) {
    va_list argp;
    va_start(argp, fmt);

    // Array for copying numbers to before converting back with scanf. It's
    // used because the packed array does not include a null byte at the end
    // of a number.
    char from[PACK_LONG_DECIMAL_SIZE+1];

    int read = 0;
    
    int number = 0;
    for (size_t i = 0; fmt[i] != '\0'; i++) {
        int digit = -1;
        memset(from, '\0', sizeof from * sizeof from[0]);

        switch (fmt[i]) {
        case 's':
            memcpy(va_arg(argp, char *), str, number);
            str += number;
            read += number;
            break;
        case 'd':
            memcpy(from, str, PACK_DECIMAL_SIZE);
            sscanf(from, "%d", va_arg(argp, int *));
            str += PACK_DECIMAL_SIZE;
            read += PACK_DECIMAL_SIZE;
            break;
        case 'l':
            memcpy(from, str, PACK_LONG_DECIMAL_SIZE);
            sscanf(from, "%lld", va_arg(argp, long long *));
            str += PACK_LONG_DECIMAL_SIZE;
            read += PACK_LONG_DECIMAL_SIZE;
            break;
        case 'b':
            memcpy(from, str, PACK_BOOL_SIZE);
            sscanf(from, "%c", va_arg(argp, char *));
            str += PACK_BOOL_SIZE;
            read += PACK_BOOL_SIZE;
            break;
        case '1': case '2': case '3': case '4': case '5':
        case '6': case '7': case '8': case '9': case '0':
            digit = fmt[i] - '0';
            break;
        default:
            fprintf(stderr, "weird pack argument\n");
            return read;
        }

        number = number * 10 + digit;
        if (digit == -1)
            number = 0;
    }

    va_end(argp);

    return read;
}

size_t fmt_length(const char *const fmt) {
    size_t len = 0;

    int number = 0;
    for (int i = 0; fmt[i] != '\0'; i++) {
        int digit = -1;

        switch (fmt[i]) {
        case 's':
            len += number;
            break;
        case 'd':
            len += PACK_DECIMAL_SIZE;
            break;
        case 'l':
            len += PACK_LONG_DECIMAL_SIZE;
            break;
        case 'b':
            len += PACK_BOOL_SIZE;
            break;
        case '1': case '2': case '3': case '4': case '5':
        case '6': case '7': case '8': case '9': case '0':
            digit = fmt[i] - '0';
            break;
        default:
            fprintf(stderr, "weird pack argument\n");
            return len;
        }

        number = number * 10 + digit;
        if (digit == -1)
            number = 0;
    }
    return len;
}

int sendall(const int sockfd, const char *const buf, const int len) {
    int total = 0;
    int bytes_left = len;
    
    while (total < len) {
        int n = send(sockfd, buf+total, bytes_left, 0);
        if (n == -1) {
            perror("sendall: send");
            return bytes_left;
        }

        total += n;
        bytes_left -= n;
    }

    return 0;
}

int recvall(const int sockfd, char *const buf, const int len) {
    int total = 0;
    int bytes_left = len;

    while (total < len) {
        int n = recv(sockfd, buf+total, bytes_left, 0);
        if (n == -1) {
            perror("recvall: recv");
            return bytes_left;
        }

        total += n;
        bytes_left -= n;
    }

    return 0;
}

void send_packet_type(const int sockfd, const PacketType type) {
    char buf[PACK_DECIMAL_SIZE];
    pack(buf, "d", type);
    sendall(sockfd, buf, PACK_DECIMAL_SIZE);
}

PacketType recv_packet_type(const int sockfd) {
    char buf[PACK_DECIMAL_SIZE];
    recvall(sockfd, buf, PACK_DECIMAL_SIZE);

    PacketType type;
    unpack(buf, "d", &type);

    return type;
}

int get_client_socket(const char *const ip, const char *const port) {
    debug("connecting to %s:%s ...", ip, port);

    struct addrinfo *servinfo;
    struct addrinfo hints = (struct addrinfo){
        .ai_family = AF_INET,
        .ai_socktype = SOCK_STREAM,
    };
    
    int ret;
    if ((ret = getaddrinfo(ip, port, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(ret));
        exit(EXIT_FAILURE);
    }

    int ret_socket = -1;

    for (struct addrinfo *p = servinfo; p != NULL; p = p->ai_next) {
        int sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (sockfd == -1) {
            perror("socket");
            continue;
        }

        ret = connect(sockfd, p->ai_addr, p->ai_addrlen);
        if (ret == -1) {
            close(sockfd);
            perror("connect");
            continue;
        }

        ret_socket = sockfd;
        break;
    }

    freeaddrinfo(servinfo);

    debug("connected!");
    return ret_socket;
}


/* main that I used for "testing":
int main() {
    char s[255] = {0};
    pack(s, "db10sl", -1234567890, true, "AAAABBBBCCCC", -1234567890123456789ll);
    long long ll;
    int d;
    bool b;
    char s2[10];
    unpack(s, "db10sl", &d, &b, s2, &ll);
    return 0;
}
*/
