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

#include "io_operations.h"
#include "socket_operations.h"
#include "pipe_operations.h"

#define FAIL (-1)
#define MAX_CLIENTS_COUNT (510)
#define WAIT_TIME (3 * 60)
#define TIMEOUT_CODE (0)
#define REQUIRED_ARGC (4)
#define USAGE_GUIDE "usage: ./prog <proxy_port> <serv_ipv4_addr> <serv_port>"
#define READ_PIPE_END (0)
#define WRITE_PIPE_END (1)
#define TERMINATE_COMMAND "stop"

int signal_pipe[2];

typedef struct args_t {
    bool valid;
    int proxy_server_port;
    char *serv_ip_address;
    int server_port;
} args_t;

static bool extract_int(const char *buf, int *num) {
    if (NULL == buf || num == NULL) {
        return false;
    }
    char *end_ptr = NULL;
    *num = (int) strtol(buf, &end_ptr, 10);
    if (buf + strlen(buf) > end_ptr) {
        return false;
    }
    return true;
}

static args_t parse_args(int argc, char *argv[]) {
    args_t result;
    result.valid = false;
    if (argc < REQUIRED_ARGC) {
        return result;
    }
    bool extracted = extract_int(argv[1], &result.proxy_server_port);
    if (!extracted) {
        return result;
    }
    result.serv_ip_address = argv[2];
    extracted = extract_int(argv[3], &result.server_port);
    if (!extracted) {
        return result;
    }
    result.valid = true;
    return result;
}

static void handle_sigint_sigterm(int sig) {
    write_into_file(signal_pipe[WRITE_PIPE_END], TERMINATE_COMMAND, strlen(TERMINATE_COMMAND));
}

int main(int argc, char *argv[]) {
    args_t args = parse_args(argc, argv);
    if (!args.valid) {
        fprintf(stderr, "%s\n", USAGE_GUIDE);
        return EXIT_FAILURE;
    }
    int return_value = pipe(signal_pipe);
    if (return_value == FAIL) {
        perror("=== Error in pipe()");
        return EXIT_FAILURE;
    }
    signal(SIGINT, handle_sigint_sigterm);
    signal(SIGTERM, handle_sigint_sigterm);
    /* This table matches client socket of a proxy server
     * and client socket of a main server */
    int translation_table[MAX_CLIENTS_COUNT * 2 + 3];
    int proxy_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (proxy_socket == FAIL) {
        perror("=== Error in socket");
        return EXIT_FAILURE;
    }
    return_value = set_reusable(proxy_socket);
    if (return_value == FAIL) {
        fprintf(stderr, "Failed to make socket reusable\n");
        return EXIT_FAILURE;
    }
    struct sockaddr_in proxy_sockaddr;
    proxy_sockaddr.sin_family = AF_INET;
    proxy_sockaddr.sin_addr.s_addr = INADDR_ANY;
    proxy_sockaddr.sin_port = htons(args.proxy_server_port);
    return_value = bind(proxy_socket, (struct sockaddr *) &proxy_sockaddr, sizeof(proxy_sockaddr));
    if (return_value < 0) {
        perror("=== Error in bind");
        return_value = close(proxy_socket);
        if (return_value == FAIL) {
            perror("=== Error in close");
        }
        return EXIT_FAILURE;
    }
    return_value = listen(proxy_socket, MAX_CLIENTS_COUNT);
    if (return_value == FAIL) {
        perror("=== Error in listen");
        return_value = close(proxy_socket);
        if (return_value == FAIL) {
            perror("=== Error in close");
        }
        return EXIT_FAILURE;
    }
    fd_set master_read_set;
    FD_ZERO(&master_read_set);
    int max_sd = proxy_socket;
    FD_SET(signal_pipe[READ_PIPE_END], &master_read_set);
    FD_SET(proxy_socket, &master_read_set); // add listen_fd to our set
    struct timeval timeout = {
            .tv_sec = WAIT_TIME,
            .tv_usec = 0
    };
    fd_set working_read_set;
    bool shutdown = false;
    printf("=== Proxy is running...\n");
    while (shutdown == false) {
        printf("=== Waiting on select...\n");
        memcpy(&working_read_set, &master_read_set, sizeof(master_read_set));
        return_value = select(max_sd + 1, &working_read_set, NULL, NULL, &timeout);
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
            if (FD_ISSET(fd, &working_read_set)) { // Check to see if this descriptor is ready
                desc_ready -= 1;
                if (fd == proxy_socket) {
                    int client_proxy_sd = accept(proxy_socket, NULL, NULL);
                    if (client_proxy_sd == FAIL) {
                        if (errno != EAGAIN) {
                            perror("=== Error in accept. Shutdown server...");
                            shutdown = true;
                        }
                        break;
                    }
                    /* make a new connection to translate data to a host server */
                    int client_main_serv_sd = make_new_connection(args.serv_ip_address, args.server_port);
                    if (client_main_serv_sd == FAIL) {
                        perror("=== Error occurred in an attempt to "
                               "establish a new connection");
                        return_value = close(client_proxy_sd);
                        if (return_value == FAIL) {
                            perror("=== Error in close");
                        }
                        continue;
                    }
                    FD_SET(client_proxy_sd, &master_read_set);
                    if (client_proxy_sd > max_sd) {
                        max_sd = client_proxy_sd;
                    }
                    FD_SET(client_main_serv_sd, &master_read_set);
                    if (client_main_serv_sd > max_sd) {
                        max_sd = client_main_serv_sd;
                    }
                    translation_table[client_proxy_sd] = client_main_serv_sd;
                    translation_table[client_main_serv_sd] = client_proxy_sd;
                } else {
                    if (fd == signal_pipe[READ_PIPE_END]) {
                        char *message = read_from_file(fd);
                        if (strcmp(message, TERMINATE_COMMAND) == 0) {
                            free(message);
                            goto FINISH;
                        }
                    }
                    char *message = read_from_socket(fd);
                    if (NULL == message) {
                        perror("=== Error in read");
                        continue;
                    }
                    if (strlen(message) == 0) {
                        // connection was closed
                        return_value = close(fd);
                        if (return_value == FAIL) {
                            perror("=== Error in close");
                        }
                        printf("=== Closed connection %d\n", fd);
                        FD_CLR(fd, &master_read_set);
                        if (fd == max_sd) {
                            max_sd -= 1;
                        }
                        free(message);
                        return_value = close(translation_table[fd]);
                        if (return_value == FAIL) {
                            perror("=== Error in close");
                        }
                        printf("=== Closed connection %d\n", translation_table[fd]);
                        FD_CLR(translation_table[fd], &master_read_set);
                        if (translation_table[fd] == max_sd) {
                            max_sd -= 1;
                        }
                        continue;
                    }
                    printf("=== Received from %d: %s    Length: %zu\n", fd, message, strlen(message));
                    bool sent = write_into_file(translation_table[fd], message, strlen(message));
                    if (!sent) {
                        perror("=== Error in write");
                    }
                    free(message);
                }
            }
        }
    }
    FINISH:
    {
        printf("\n=== Shutdown proxy server...\n");
        for (int sock_fd = 0; sock_fd <= max_sd; ++sock_fd) {
            if (FD_ISSET(sock_fd, &master_read_set)) {
                return_value = close(sock_fd);
                if (return_value == FAIL) {
                    perror("=== Error in close");
                }
            }
        }
    }
}
