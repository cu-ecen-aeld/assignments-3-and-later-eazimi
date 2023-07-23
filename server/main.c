#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h> 
#include <syslog.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "aesdsocket.h"
#include "handlefiles.h"

#define PORT 9000
#define BUFF_SIZE 1024
#define FILE_PATH "/var/tmp/aesdsocketdata"

#define CHECK_EXIT_CONDITION(rc, func_name) do { \
    if((rc) == -1) \
    { \
        fprintf(stderr, "%s error: %s", (func_name), strerror(errno)); \
        exit(EXIT_FAILURE); \
    } \
} while(0)

int main(int argc, char **argv)
{
    openlog("server_log", LOG_PID, LOG_USER);
    int rc_clearfile = clear_file(FILE_PATH);
    CHECK_EXIT_CONDITION(rc_clearfile, "clear_file");

    int sockfd = create_socket();
    CHECK_EXIT_CONDITION(sockfd, "create_socket");

    char port[5];
    memset(port, 0, sizeof port);
    sprintf(port, "%d", PORT);
    int rc_bind = bind_addr(sockfd, port);
    CHECK_EXIT_CONDITION(rc_bind, "bind_addr");

    int rc_listen = listen_conn(sockfd);
    CHECK_EXIT_CONDITION(rc_listen, "listen_conn");

    struct sockaddr addr_cli;
    int connfd = accept_conn(sockfd, &addr_cli);
    CHECK_EXIT_CONDITION(connfd, "accept_conn");

    struct sockaddr_in addr;
    int rc_getpeername = getpeername(sockfd, (struct sockaddr *)&addr, (socklen_t *)sizeof(struct sockaddr));
    CHECK_EXIT_CONDITION(rc_getpeername, "getpeername");
    syslog(LOG_INFO, "Accepted connection from %s", inet_ntoa(addr.sin_addr));

    char recv_buff[BUFF_SIZE];
    memset((void *)recv_buff, 0, BUFF_SIZE);
    int rc_recvdata = recv_data(connfd, recv_buff, BUFF_SIZE);
    CHECK_EXIT_CONDITION(rc_recvdata, "recv_data");

    int pfd = open_file(FILE_PATH);
    CHECK_EXIT_CONDITION(pfd, "open_file");

    int rc_writefile = write_file(pfd, (const void*)recv_buff, strlen(recv_buff));
    CHECK_EXIT_CONDITION(rc_writefile, "write_file");

    char send_buff[BUFF_SIZE];
    memset((void *)send_buff, 0, BUFF_SIZE);
    int rc_readfile = read_file(pfd, send_buff, BUFF_SIZE);
    CHECK_EXIT_CONDITION(rc_readfile, "read_file");

    int rc_senddata = send_data(connfd, send_buff, rc_readfile);
    CHECK_EXIT_CONDITION(rc_senddata, "send_data");
    
    close_socket(sockfd);
    closelog();
    close_file(pfd);

    return 0;
}