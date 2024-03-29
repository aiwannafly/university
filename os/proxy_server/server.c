#include <arpa/inet.h>
#include <errno.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/un.h>
#include <unistd.h>
#include <ctype.h>

#include "io_operations.h"
#include "socket_operations.h"
#include "pipe_operations.h"

#define FAIL (-1)
#define MAX_CLIENTS_COUNT (32)
#define WAIT_TIME (3 * 60)
#define TIMEOUT_CODE (0)
#define STOP_MESSAGE "exit\n"
#define SERVER_PORT (5010)
#define READ_PIPE_END (0)
#define WRITE_PIPE_END (1)
#define TERMINATE_COMMAND "stop"
#define IPV4_SERV_ADDRESS "127.0.0.1"

int signal_pipe[2];

static void handle_sigint_sigterm(int sig) {
    write_into_file(signal_pipe[WRITE_PIPE_END], TERMINATE_COMMAND, strlen(TERMINATE_COMMAND));
}

int main() {
    signal(SIGINT, handle_sigint_sigterm);
    signal(SIGTERM, handle_sigint_sigterm);
    int serv_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (serv_socket == FAIL) {
        perror("=== Error in socket");
        return EXIT_FAILURE;
    }
    int return_value = set_reusable(serv_socket);
    if (return_value == FAIL) {
        fprintf(stderr, "Failed to make socket reusable\n");
        return EXIT_FAILURE;
    }
    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_port = htons(SERVER_PORT);
    return_value = inet_pton(AF_INET, IPV4_SERV_ADDRESS, &address.sin_addr);
    if (return_value == FAIL) {
        perror("=== Error in inet_pton");
        return EXIT_FAILURE;
    }
    return_value = bind(serv_socket, (struct sockaddr *) &address, sizeof(address));
    if (return_value < 0) {
        perror("=== Error in bind");
        return_value = close(serv_socket);
        if (return_value == FAIL) {
            perror("=== Error in close");
        }
        return EXIT_FAILURE;
    }
    return_value = listen(serv_socket, MAX_CLIENTS_COUNT);
    if (return_value == FAIL) {
        perror("=== Error in listen");
        return_value = close(serv_socket);
        if (return_value == FAIL) {
            perror("=== Error in close");
        }
        return EXIT_FAILURE;
    }
    fd_set master_set;
    FD_ZERO(&master_set);
    int max_sd = serv_socket;
    FD_SET(serv_socket, &master_set); // add listen_fd to our set
    FD_SET(signal_pipe[READ_PIPE_END], &master_set);
    struct timeval timeout = {
            .tv_sec = WAIT_TIME,
            .tv_usec = 0
    };
    fd_set working_set;
    bool shutdown = false;
    printf("=== Server is running...\n");
    while (shutdown == false) {
        printf("=== Waiting on select...\n");
        memcpy(&working_set, &master_set, sizeof(master_set));
        return_value = select(max_sd + 1, &working_set, NULL, NULL, &timeout);
        if (return_value == FAIL) {
            if (errno != EINTR) {
                perror("=== Error in select");
            }
            break;
        }
        if (return_value == TIMEOUT_CODE) {
            fprintf(stderr, "=== Select timed out. End program.\n");
            break;
        }
        int desc_ready = return_value;
        for (int fd = 0; fd <= max_sd && desc_ready > 0; ++fd) {
            if (FD_ISSET(fd, &working_set)) { // Check to see if this descriptor is ready
                desc_ready -= 1;
                printf("sock_fd: %d\n", fd);
                if (fd == serv_socket) {
                    int new_client_sock = accept(serv_socket, NULL, NULL);
                    if (new_client_sock == FAIL) {
                        if (errno != EAGAIN) {
                            perror("=== Error in accept. Shutdown server...\n");
                            shutdown = true;
                        }
                        break;
                    }
                    printf("=== Accepted new connection...\n");
                    FD_SET(new_client_sock, &master_set);
                    if (new_client_sock > max_sd) {
                        max_sd = new_client_sock;
                    }
                } else {
                    if (fd == signal_pipe[READ_PIPE_END]) {
                        char *message = read_from_file(fd);
                        if (strcmp(message, TERMINATE_COMMAND) == 0) {
                            goto FINISH;
                        }
                    }
                    printf("=== Reading from %d...\n", fd);
                    char *message = read_from_socket(fd);
                    if (NULL == message) {
                        perror("=== Error in read");
                        continue;
                    }
                    if (strcmp(STOP_MESSAGE, message) == 0 || strlen(message) == 0) {
                        return_value = close(fd);
                        if (return_value == FAIL) {
                            perror("=== Error in close");
                        }
                        FD_CLR(fd, &master_set);
                        if (fd == max_sd) {
                            max_sd -= 1;
                        }
                        printf("=== Closed connection %d\n", fd);
                        free(message);
                        continue;
                    }
                    printf("----> Client %d: ", fd - 3);
                    for (size_t i = 0; i < strlen(message); i++) {
                        message[i] = (char) toupper(message[i]);
                        printf("%c", message[i]);
                    }
                    bool sent_reply = write_into_file(fd, message, strlen(message));
                    if (!sent_reply) {
                        perror("=== Error in write");
                    }
                    free(message);
                }
            }
        }
    }
    FINISH:
    {
        printf("\n=== Shutdown server...\n");
        for (int sock_fd = 0; sock_fd <= max_sd; ++sock_fd) {
            if (FD_ISSET(sock_fd, &master_set)) {
                return_value = close(sock_fd);
                if (return_value == FAIL) {
                    perror("=== Error in close");
                }
            }
        }
    }
}
