#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h> 
#include <syslog.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include "aesdsocket.h"
#include "handlefiles.h"

#define PORT 9000
#define BUFF_SIZE 1024
#define PCKT_SIZE 20*1024 
#define FILE_PATH "/var/tmp/aesdsocketdata"

bool accept_conn_loop = true;

#define CHECK_EXIT_CONDITION(rc, func_name) do { \
    if((rc) == -1) \
    { \
        fprintf(stderr, "%s error: %s\n", (func_name), strerror(errno)); \
        exit(EXIT_FAILURE); \
    } \
} while(0)

static void signal_handler(int signal_number)
{
    if((signal_number == SIGINT) || (signal_number == SIGTERM))
    {
        fprintf(stdout, "SIGTERM/SIGINT received\n");
        accept_conn_loop = false;
    }
}

bool split_buffer(const char *input, char *buff1, char *buff2)
{
    char *pch = strstr(input, "\n");
    if(pch == NULL)
    {
        buff1[0] = '\0';
        buff2[0] = '\0';
        return false;
    }

    strncpy(buff2, pch + 1, input + BUFF_SIZE - pch - 1);
    *(buff2 + (input + BUFF_SIZE - pch - 1)) = '\0';

    strncpy(buff1, input, pch - input + 1);
    *(buff1 + (pch - input + 1)) = '\0';
    return true;
}

int main(int argc, char **argv)
{
    char *packet = (char *)malloc(PCKT_SIZE);
    memset(packet, 0, PCKT_SIZE);
    int packet_index = 0;

    openlog("server_log", LOG_PID, LOG_USER);
    int rc_clearfile = clear_file(FILE_PATH);
    CHECK_EXIT_CONDITION(rc_clearfile, "clear_file");

    struct sigaction new_action;
    memset((void *)&new_action, 0, sizeof(struct sigaction));
    new_action.sa_handler = signal_handler;
    bool success = true;
    if(sigaction(SIGTERM, &new_action, NULL) != 0)
    {
        fprintf(stderr, "error %d (%s) registering for SIGTERM\n", errno, strerror(errno));
        success = false;
    }
    if(sigaction(SIGINT, &new_action, NULL) != 0)
    {
        fprintf(stderr, "error %d (%s) registering for SIGINT\n", errno, strerror(errno));
        success = false;
    }

    int pid = -1;
    if(success)
    {
        pid = fork();
        CHECK_EXIT_CONDITION(pid, "fork");
        if(pid == 0)
        {
            pause();
        }
    }

    //////////////////////////////// socket
    int sockfd = create_socket();
    CHECK_EXIT_CONDITION(sockfd, "create_socket");

    char port[5];
    memset(port, 0, sizeof port);
    sprintf(port, "%d", PORT);
    int rc_bind = bind_addr(sockfd, port);
    CHECK_EXIT_CONDITION(rc_bind, "bind_addr");

    int rc_listen = listen_conn(sockfd);
    CHECK_EXIT_CONDITION(rc_listen, "listen_conn");

    while (accept_conn_loop)
    {
        struct sockaddr addr_cli;
        fprintf(stdout, "waiting to accept a connection ...\n");
        int connfd = accept_conn(sockfd, &addr_cli);
        CHECK_EXIT_CONDITION(connfd, "accept_conn");

        struct sockaddr_in addr;
        int rc_getpeername = getpeername(sockfd, (struct sockaddr *)&addr, (socklen_t *)sizeof(struct sockaddr));
        CHECK_EXIT_CONDITION(rc_getpeername, "getpeername");
        syslog(LOG_INFO, "Accepted connection from %s", inet_ntoa(addr.sin_addr));

        //////////////////////////////// receive data - write to file
        char recv_buff[BUFF_SIZE];
        memset((void *)recv_buff, 0, BUFF_SIZE);
        int rc_recvdata = recv_data(connfd, recv_buff, BUFF_SIZE);
        CHECK_EXIT_CONDITION(rc_recvdata, "recv_data");

        char buff1[BUFF_SIZE + 1], buff2[BUFF_SIZE + 1];
        bool newpacket = split_buffer(recv_buff, buff1, buff2);
        if (newpacket)
        {
            memcpy((void *)(&packet[packet_index]), buff1, sizeof(buff1));
            int pfd = open_file(FILE_PATH);
            CHECK_EXIT_CONDITION(pfd, "open_file");

            int rc_writefile = write_file(pfd, (const void *)packet, sizeof(packet));
            CHECK_EXIT_CONDITION(rc_writefile, "write_file");
            close_file(pfd);

            memset(packet, 0, PCKT_SIZE);
            packet_index = 0;
            // packet_index = strlen(buff2);
            // strncpy(packet, buff2, packet_index);

            //////////////////////////////// read data from file - send it back
            char send_buff[BUFF_SIZE];
            memset((void *)send_buff, 0, BUFF_SIZE);
            pfd = open_file(FILE_PATH);
            CHECK_EXIT_CONDITION(pfd, "open_file/send_data");
            int rc_readfile = -1;
            do
            {
                rc_readfile = read_file(pfd, send_buff, BUFF_SIZE);
                CHECK_EXIT_CONDITION(rc_readfile, "read_file");
                int rc_senddata = send_data(connfd, send_buff, rc_readfile);
                CHECK_EXIT_CONDITION(rc_senddata, "send_data");
            } while (rc_readfile > 0);
            close_file(pfd);

            syslog(LOG_INFO, "Closed connection from %s", inet_ntoa(addr.sin_addr));
            close_socket(connfd);
        }
        else
        {
            memcpy((void *)(&packet[packet_index]), recv_buff, BUFF_SIZE);
            packet_index += BUFF_SIZE;
        }
    }

    //////////////////////////////// shutdown
    close_socket(sockfd);
    closelog();
    remove(FILE_PATH);
    kill(pid, SIGINT);

    return 0;
}