#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>
#include <syslog.h>
#include <signal.h>
#include <unistd.h>
#include "aesdsocket.h"
#include "handlefiles.h"

#define PORT 9000
#define BUFF_SIZE 1024
#define PCKT_SIZE 20 * 1024
#define FILE_PATH "/var/tmp/aesdsocketdata"

bool accept_conn_loop = true;

#define CHECK_EXIT_CONDITION(rc, func_name) \
    do                                      \
    {                                       \
        if ((rc) == -1)                     \
        {                                   \
            exit(EXIT_FAILURE);             \
        }                                   \
    } while (0)

static void signal_handler(int signal_number)
{
    if ((signal_number == SIGINT) || (signal_number == SIGTERM))
    {
        syslog(LOG_INFO, "Caught signal, exiting");
        accept_conn_loop = false;
    }
}

void _daemon()
{
    // PID: Process ID
    // SID: Session ID
    pid_t pid, sid;
    pid = fork(); // Fork off the parent process
    if (pid < 0)
    {
        exit(EXIT_FAILURE);
    }
    if (pid > 0)
    {
        exit(EXIT_SUCCESS);
    }
    // Create a SID for child
    sid = setsid();
    if (sid < 0)
    {
        // FAIL
        exit(EXIT_FAILURE);
    }
    if ((chdir("/")) < 0)
    {
        // FAIL
        exit(EXIT_FAILURE);
    }
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);
}

int main(int argc, char **argv)
{
    /// create socket
    int sockfd = create_socket();
    CHECK_EXIT_CONDITION(sockfd, "create_socket");

    char port[5];
    memset(port, 0, sizeof port);
    sprintf(port, "%d", PORT);
    int rc_bind = bind_addr(sockfd, port);
    CHECK_EXIT_CONDITION(rc_bind, "bind_addr");

    openlog("server_log", LOG_PID, LOG_USER);

    struct sigaction new_action;
    memset((void *)&new_action, 0, sizeof(struct sigaction));
    new_action.sa_handler = signal_handler;
    if ((sigaction(SIGTERM, &new_action, NULL) != 0) || (sigaction(SIGINT, &new_action, NULL) != 0))
    {
        return 0;
    }

    if (argc == 2 && strcmp(argv[1], "-d") == 0)
    {
        _daemon();
    }

    int rc_listen = listen_conn(sockfd);
    CHECK_EXIT_CONDITION(rc_listen, "listen_conn");

    int pfd = open_file(FILE_PATH);
    CHECK_EXIT_CONDITION(pfd, "open_file");

    while (accept_conn_loop)
    {
        struct sockaddr addr_cli;
        int connfd = accept_conn(sockfd, &addr_cli);
        if (connfd == -1)
        {
            shutdown(sockfd, SHUT_RDWR);
            continue;
        }

        char str_ipcli[BUFF_SIZE];
        get_ipcli(&addr_cli, str_ipcli);
        syslog(LOG_INFO, "Accepted connection from %s", str_ipcli);

        while (true)
        {
            /// receive data - write to file
            char recv_buff[BUFF_SIZE + 1];
            memset((void *)recv_buff, 0, BUFF_SIZE + 1);
            int rc_recvdata = recv_data(connfd, recv_buff, BUFF_SIZE);
            CHECK_EXIT_CONDITION(rc_recvdata, "recv_data");

            int rc_writefile = write_file(pfd, (const void *)recv_buff, rc_recvdata);
            CHECK_EXIT_CONDITION(rc_writefile, "write_file");

            char *pch = strstr(recv_buff, "\n");
            if (pch != NULL)
                break;
        }

        int data_size = lseek(pfd, 0L, SEEK_CUR);
        char send_buff[BUFF_SIZE];
        lseek(pfd, 0L, SEEK_SET);
        do
        {
            int rc_readfile = read_file(pfd, send_buff, BUFF_SIZE);
            CHECK_EXIT_CONDITION(rc_readfile, "read_file");
            int rc_senddata = send_data(connfd, send_buff, rc_readfile);
            CHECK_EXIT_CONDITION(rc_senddata, "send_data");
            data_size -= rc_readfile;
            memset(send_buff, 0, BUFF_SIZE);
        } while (data_size > 0);

        syslog(LOG_INFO, "Closed connection from %s", str_ipcli);
    }

    /// shutdown
    close(pfd);
    closelog();
    remove(FILE_PATH);

    return 0;
}