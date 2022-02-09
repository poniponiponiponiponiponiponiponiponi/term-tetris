#include <arpa/inet.h>
#include <netdb.h>
//#include <netinet/in.h>
#include <poll.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "multiplayer.h"
#include "server.h"
#include "util.h"

void sigchld_handler() {
    while (waitpid(-1, NULL, WNOHANG) > 0)
        ;
}

void reap_zombies(void) {
    struct sigaction sa = (struct sigaction){
        .sa_handler = sigchld_handler,
        .sa_flags = SA_RESTART
    };
    sigemptyset(&sa.sa_mask);

    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }
}

int get_listening_socket(void) {
    struct addrinfo *servinfo;
    struct addrinfo hints = (struct addrinfo){
        .ai_family = AF_INET,
        .ai_socktype = SOCK_STREAM,
        .ai_flags = AI_PASSIVE
    };
    
    int ret;
    if ((ret = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0) {
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

        int yes = 1;
        ret = setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof yes);
        if (ret == -1) {
            perror("setsockopt");
            close(sockfd);
            continue;
        }

        ret = bind(sockfd, p->ai_addr, p->ai_addrlen);
        if (ret == -1) {
            perror("bind");
            close(sockfd);
            continue;
        }

        ret = listen(sockfd, 10);
        if (ret == -1) {
            perror("listen");
            close(sockfd);
            continue;
        }

        ret_socket = sockfd;
        break;
    }

    freeaddrinfo(servinfo);

    return ret_socket;
}

void wait_for_players(
        int sockfd,
        size_t len,
        Player *players,
        struct pollfd *pfds) {
    printf("server: waiting for players...\n");

    Player player = 1;
    Player max_player = 2;

    if (players[0] != 0) {
        printf("server: player 1 is already connected.\n");
        player = 2;
    }
    if (players[1] != 0) {
        printf("server: player 2 is already connected.\n");
        max_player = 1;
    }

    while (player <= max_player) {
        struct sockaddr_storage their_addr;
        socklen_t sin_size = sizeof their_addr;

        int new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
        if (new_fd == -1) {
            perror("accept");
            continue;
        }

        char s[INET6_ADDRSTRLEN];
        inet_ntop(
                their_addr.ss_family,
                &((struct sockaddr_in *)&their_addr)->sin_addr,
                s, sizeof s);
        printf("server: player %d: got connection from: %s\n", player, s);
        
        players[player++-1] = new_fd;
    }

    for (size_t i = 0; i < len; i++) {
        pfds[i] = (struct pollfd) {
            .fd = players[i],
            .events = POLLIN
        };
    }

    printf("server: all players are connected.\n");
    for (size_t i = 0; i < len; i++)
        send_packet_type(players[i], MULTI_START);
}

int broadcast(
        const int source_fd,
        const size_t len,
        const int *const sock_fds,
        const int buf_len,
        const char *buf) {
    for (size_t i = 0; i < len; i++) {
        const int dest_fd = sock_fds[i];

        if (dest_fd != source_fd) {
            int nbytes = send(dest_fd, buf, buf_len, 0);
            if (nbytes == -1) {
                perror("send");
            }
        }
    }
    return 0;
}

int main(void) {
    int sockfd = get_listening_socket();
    if (sockfd == -1) {
        fprintf(stderr, "failed to connect to socket\n");
        exit(EXIT_FAILURE);
    }

    reap_zombies();

    Player players[2] = {0};
    const size_t players_n = ARRAY_SIZE(players);
    struct pollfd pfds[players_n];

    wait_for_players(sockfd, players_n, players, pfds);

    // main loop
    for (;;) {
        int poll_cnt = poll(pfds, players_n, -1);
        if (poll_cnt == -1) {
            perror("poll");
            exit(EXIT_FAILURE);
        }

        for (size_t i = 0; i < players_n; i++) {
            if ((pfds[i].revents & POLLIN) == 0)
                continue;
            if (poll_cnt <= 0)
                break;

            poll_cnt--;
            char buf[4096] = {0};

            int nbytes = recv(pfds[i].fd, buf, sizeof buf, 0);

            if (nbytes <= 0) {
                // take care of an error
                close(pfds[i].fd);
                players[i] = 0;
                if (nbytes == 0) {
                    printf("server: socket %d closed\n", pfds[i].fd);
                    for (size_t j = 0; j < players_n; j++)
                        if (i != j)
                            send_packet_type(players[j], MULTI_PAUSE);
                    wait_for_players(sockfd, players_n, players, pfds);
                } else {
                    perror("recv");
                }
            } else {
                broadcast(players[i], players_n, players, nbytes, buf);
            }
        }
    }

    return 0;
}
