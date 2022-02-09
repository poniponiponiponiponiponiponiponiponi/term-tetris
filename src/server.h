#ifndef SERVER_H
#define SERVER_H

void sigchld_handler();
void reap_zombies(void);
int get_listening_socket(void);
void wait_for_players(
        int sockfd,
        size_t len,
        Player *players,
        struct pollfd *pfds);
int broadcast(
        const int source_fd,
        const size_t len,
        const int *const sock_fds,
        const int buf_len,
        const char *buf);

#endif
