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
#define RECV_SIZE 1024
#define FILE_PATH "/var/tmp/aesdsocketdata"

int main(int argc, char **argv)
{
    openlog("server_log", LOG_PID, LOG_USER);

    int sockfd = create_socket();
    if(sockfd == -1)
    {
        fprintf(stderr, "socket error: %s", strerror(errno));
        return -1;
    }

    char port[5];
    memset(port, 0, sizeof port);
    sprintf(port, "%d", PORT);
    int rc_bind = bind_addr(sockfd, port);
    if(rc_bind == -1)
    {
        fprintf(stderr, "bind_addr error: %s", strerror(errno));
        return -1;
    }

    int rc_listen = listen_conn(sockfd);
    if(rc_listen == -1)
    {
        fprintf(stderr, "listen_conn error: %s", strerror(errno));
        return -1;
    }

    struct sockaddr addr_cli;
    int connfd = accept_conn(sockfd, &addr_cli);
    if(connfd == -1)
    {
        fprintf(stderr, "accept_conn error: %s", strerror(errno));
        return -1;
    }

    struct sockaddr_in addr;
    int rc_getpeername = getpeername(sockfd, (struct sockaddr *)&addr, (socklen_t *)sizeof(struct sockaddr));
    if(rc_getpeername == -1)
    {
        fprintf(stderr, "getpeername error: %s", strerror(errno));
        return -1;
    }
    syslog(LOG_INFO, "Accepted connection from %s", inet_ntoa(addr.sin_addr));

    char recv_buff[RECV_SIZE];
    memset((void *)recv_buff, 0, RECV_SIZE);
    int rc_recvdata = recv_data(connfd, recv_buff, RECV_SIZE);
    if(rc_recvdata == -1)
    {
        fprintf(stderr, "recv_data error: %s", strerror(errno));
        return -1;
    }

    int pfd = open_file(FILE_PATH);
    if(pfd == -1)
    {
        fprintf(stderr, "open_file error: %s", strerror(errno));
        return -1;
    }

    int rc_writefile = write_file(pfd, (const void*)recv_buff, strlen(recv_buff));
    if(rc_writefile == -1)
    {
        fprintf(stderr, "write_file error: %s", strerror(errno));
        return -1;
    }
    
    close_socket(sockfd);
    closelog();
    close_file(pfd);

    return 0;
}